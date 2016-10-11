<?php
/**
 * @file
 * This file contains the functions used for administration of the Breeding API
 * module.
 *
 * @ingroup brapi
 */
 
/**
 * Administrative settings form.
 *
 * @return string
 *   the HTML content of the administration form.
 *
 * @ingroup brapi
 */
function brapi_admin_form() {
  $form = array();

  $form['cv_settings'] = array(
    '#type'        => 'fieldset', 
    '#attributes'  => array('class' => array('container-inline')),
    '#title'       => t('Controlled vocabulary settings'), 
    '#collapsible' => TRUE, 
    '#collapsed'   => FALSE,
    '#suffix'      => '<br/>',
  );

  // CV settings
  $brapi_cv_select = array('name' => BRAPI_CV);
  $brapi_cv = chado_select_record('cv', array('*'), $brapi_cv_select);
  if (is_array($brapi_cv)) {
    $brapi_cv = current($brapi_cv);
  }

  $brapi_cv_settings = brapi_get_cv_settings();
  foreach (brapi_get_cv() as $term_name => $definition) {
    $default_id = 0;
    
    // get from settings
    if (array_key_exists($term_name, $brapi_cv_settings)) {
      $default_id = $brapi_cv_settings[$term_name];
      $term = tripal_get_cvterm(array('cvterm_id' => $default_id,));
    }
    
    // get term from BrAPI CV
    if (!$term) {
      $term =
        tripal_get_cvterm(
          array('name' => $term_name, 'cv_id' => $brapi_cv->cv_id,)
        );
    }

    if ($term) {
      if (is_array($term)) {
        $term = current($term);
      }
      $default_id =
        $term->name
        . ' (cv:' . $term->cv_id->name . ', id:' . $term->cvterm_id . ')';
    }

    $form['cv_settings'][$term_name] = array(
      '#type' => 'textfield',
      '#description' => t($definition),
      '#default_value' => $default_id,
      '#autocomplete_path' => 'brapi/terms/autocomplete',
      '#title' => t($term_name),
      '#title_display'=> 'before',
      '#required' => TRUE,
      '#size' => 80,
      '#field_suffix' => '<br/>&nbsp;&nbsp;&nbsp;',
      '#suffix' => '<br/><br/>',
    );
  }
  
  // Default example entities settings
  $example_parameters = array();
  // get a unique list of all available parameters
  foreach (brapi_get_calls() as $call) {
    if (array_key_exists('arguments', $call)) {
      foreach ($call['arguments'] as $param) {
        $example_parameters[$param['name']] = $param;
      }
    }
  }

  if (count($example_parameters)) {
    $form['spacer'] = array(
      '#value' => '<br/>',
    );
    $form['example_settings'] = array(
      '#type'        => 'fieldset', 
      '#attributes'  => array('class' => array('container-inline')),
      '#title'       => t('Example value settings'), 
      '#collapsible' => TRUE, 
      '#collapsed'   => FALSE,
      '#suffix'      => '<br/>',
    );
    ksort($example_parameters);
  }

  $brapi_example_settings = variable_get(BRAPI_EXAMPLE_SETTINGS);

  foreach ($example_parameters as $example_parameter) {
    $form['example_settings'][$example_parameter['name']] = array(
      '#type' => 'textfield',
      '#description' => t($example_parameter['description']),
      '#default_value' => (
        array_key_exists($example_parameter['name'], $brapi_example_settings) ?
          $brapi_example_settings[$example_parameter['name']]
          : ''
      ),
      '#title' => $example_parameter['name'],
      '#title_display'=> 'before',
      '#required' => FALSE,
      '#size' => 80,
      '#field_suffix' => '<br/>&nbsp;&nbsp;&nbsp;',
      '#suffix' => '<br/><br/>',
    );
  }
  
  return system_settings_form($form);
}

/**
 * Check and applies Breeding API configuration changes.
 *
 * It updates Drupal variables BRAPI_CV_SETTINGS and BRAPI_EXAMPLE_SETTINGS. See
 * Breeding API constants to get the real variable names.
 *
 * @see brapi_const
 *
 * @ingroup brapi
 */
function brapi_admin_form_validate($form, &$form_state) {

  $brapi_cv_settings = brapi_get_cv_settings();
  foreach (brapi_get_cv() as $term_label => $definition) {
    if (preg_match('/^\s*(\d+)\s*$/', $form_state['values'][$term_label], $matches)) {
      // get cvterm_id
      $brapi_cv_settings[$term_label] = $matches[1];
    }
    else if (preg_match('/\Wid:\s*(\d+)/', $form_state['values'][$term_label], $matches)) {
      // get cvterm_id
      $brapi_cv_settings[$term_label] = $matches[1];
    }
    else if (preg_match('/(.+)\((?:.*\W|)cv:\s*([\w ]+)\)/', $form_state['values'][$term_label], $matches)) {
      // find term from cv
      $term_name = trim($matches[1]);
      $cv_name = trim($matches[2]);
      $term = tripal_get_cvterm(array('name' => $term_name, 'cv_id' => array('name' => $cv_name),));
      if (is_array($term)) {
        $term = current($term);
      }

      if ($term) {
        $brapi_cv_settings[$term_label] = $term->cvterm_id;
      }
      else {
        drupal_set_message('Term not found for ' . $term_label . ': "' . $term_name . '" from CV "' . $cv_name .'"', 'warning');
      }
    }
    else {
      // find term
      $raw_value = $form_state['values'][$term_label];
      // we remove extra invalid (no id: or cv:) data in parenthesis if some
      // note: if we have a parenthesis matching at position 0, it means we
      // don't have anything before so we should not remove anything then
      if ($parenthesis_pos = strpos($raw_value, '(')) {
        $raw_value = substr($raw_value, 0, $parenthesis_pos);
      }
      $term_name = trim($raw_value);
      $term = tripal_get_cvterm(array('name' => $term_name,));
      if (is_array($term)) {
        $term = current($term);
      }

      if ($term) {
        $brapi_cv_settings[$term_label] = $term->cvterm_id;
      }
      else {
        drupal_set_message('Term not found for ' . $term_label . ': "' . $term_name . '"', 'warning');
      }
    }
  }
  
  variable_set(BRAPI_CV_SETTINGS, $brapi_cv_settings);
  $brapi_cv_settings = brapi_get_cv_settings(TRUE);

  // Default example entities settings
  $example_parameters = array();
  // get a unique list of all available parameters
  foreach (brapi_get_calls() as $call) {
    if (array_key_exists('arguments', $call)) {
      foreach ($call['arguments'] as $param) {
        $example_parameters[$param['name']] = $param;
      }
    }
  }
  
  $brapi_example_settings = array();
  foreach ($example_parameters as $example_parameter) {
    $brapi_example_settings[$example_parameter['name']] = $form_state['values'][$example_parameter['name']];
  }
  variable_set(BRAPI_EXAMPLE_SETTINGS, $brapi_example_settings);
}
