<?php

/**
 * @file
 * Contains the functions used for administration of the Breeding API module.
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
function brapi_admin_form($form, &$form_state) {
  $form = array();

  $form['cv_settings'] = array(
    '#type'        => 'fieldset',
    '#attributes'  => array('class' => array('container-inline')),
    '#title'       => t('Controlled vocabulary settings'),
    '#collapsible' => TRUE,
    '#collapsed'   => FALSE,
    '#suffix'      => '<br/>',
  );

  // CV settings.
  $brapi_cv_select = array('name' => BRAPI_CV);
  $brapi_cv = chado_select_record('cv', array('*'), $brapi_cv_select);
  if (is_array($brapi_cv)) {
    $brapi_cv = current($brapi_cv);
  }

  $brapi_cv_settings = brapi_get_cv_settings();
  foreach (brapi_get_cv() as $term_name => $definition) {
    $default_id = 0;

    // Get from settings.
    if (array_key_exists($term_name, $brapi_cv_settings)) {
      $default_id = $brapi_cv_settings[$term_name];
      $term = tripal_get_cvterm(array('cvterm_id' => $default_id));
    }

    // Get term from BrAPI CV.
    if (!$term) {
      $term =
        tripal_get_cvterm(
          array('name' => $term_name, 'cv_id' => $brapi_cv->cv_id)
        );
    }

    if ($term) {
      $default_id =
        $term->name
        . ' (cv:' . $term->cv_id->name . ', id:' . $term->cvterm_id . ')';
    }

    $form['cv_settings'][$term_name] = array(
      '#type' => 'textfield',
      '#description' => $definition,
      '#default_value' => $default_id,
      '#autocomplete_path' => 'brapi/terms/autocomplete',
      '#title' => $term_name,
      '#title_display' => 'before',
      '#required' => TRUE,
      '#size' => 80,
      '#field_suffix' => '<br/>&nbsp;&nbsp;&nbsp;',
      '#suffix' => '<br/><br/>',
    );
  }

  // Default example entities settings.
  $example_parameters = array();
  // Get a unique list of all available parameters.
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
      '#description' => $example_parameter['description'],
      '#default_value' => (
        array_key_exists($example_parameter['name'], $brapi_example_settings) ?
          $brapi_example_settings[$example_parameter['name']]
          : ''
      ),
      '#title' => $example_parameter['name'],
      '#title_display' => 'before',
      '#required' => FALSE,
      '#size' => 80,
      '#field_suffix' => '<br/>&nbsp;&nbsp;&nbsp;',
      '#suffix' => '<br/><br/>',
    );
  }

  $form['chado_mcpd'] = array(
    '#type'        => 'fieldset',
    '#attributes'  => array('class' => array('container-inline')),
    '#title'       => t('MCPD settings'),
    '#collapsible' => TRUE,
    '#collapsed'   => TRUE,
    '#suffix'      => '<br/>',
  );

  $form['chado_mcpd']['reset_mcpd'] = array(
    '#type'        => 'submit',
    '#value'       => t('Reload Chado MCPD CV'),
    '#submit'      => array('brapi_reset_mcpd_form'),
  );


  $form['save_settings'] = array(
    '#type'        => 'submit',
    '#value'       => t('Save configuration'),
    '#submit'      => array('brapi_admin_form_submit'),
  );

  return $form;
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
function brapi_admin_form_submit($form_id, &$form_state) {
  $brapi_cv_settings = brapi_get_cv_settings();
  foreach (brapi_get_cv() as $term_label => $definition) {
    $user_term = $form_state['values'][$term_label];
    $cvterm_id = brapi_get_cvterm_id($user_term, FALSE);
    if ($cvterm_id) {
      $brapi_cv_settings[$term_label] = $cvterm_id;
    }
    else {
      drupal_set_message('Term not found: "' . $term_label . '"', 'warning');
    }
  }

  variable_set(BRAPI_CV_SETTINGS, $brapi_cv_settings);
  $brapi_cv_settings = brapi_get_cv_settings(TRUE);

  // Default example entities settings.
  $example_parameters = array();
  // Get a unique list of all available parameters.
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

/**
 * MCPD Reset: redirects to confirmation form.
 */
function brapi_reset_mcpd_form($form_id, &$form_state) {
  $form_state['redirect'] = 'admin/tripal/extension/brapi/configuration/reset_mcpd';
}

/**
 * MCPD reset confirmation form.
 */
function brapi_reset_mcpd_form_confirm($form_id, &$form_state) {
  $title = t('Are you sure you want to reload MCPD CV?');
  $details = t(
    'Are you sure you want to reload MCPD controlled vocabulary stored into Chado and use the version @mcpd_version provided with this module instead? If the MCPD CV already exists in your Chado instance, it will be updated to MCPD @mcpd_version but existing terms not present in @mcpd_version will be kept. Any association with existing terms will be kept while those terms might be updated. This action cannot be undone.',
    array('@mcpd_version' => BRAPI_MCPD_VERSION)
  );
  $form = array('confirm' =>array('#type' => 'hidden', '#value' => 'true'));

  return confirm_form(
    $form,
    $title,
    'admin/tripal/extension/brapi/configuration',
    $details,
    t('Reload MCPD'),
    t('Cancel')
  );
}

/**
 * MCPD rested confirmed, reload MCPD CV.
 */
function brapi_reset_mcpd_form_confirm_submit($form_id, &$form_state) {
  if (isset($form_state['values']['confirm'])) {
    $newcvs = array();
    $mcpd_path = drupal_get_path('module', 'brapi') . '/cv/mcpd_v2.1_151215.obo';
    $loaded = tripal_cv_load_obo_v1_2($mcpd_path, NULL, $newcvs);
    if ($loaded && !empty($newcvs)) {
      drupal_set_message('MCPD CV loaded or updated successfully.');
    }
    else {
      drupal_set_message('Failed to load or update MCPD CV!', 'warning');
    }
  }

  $form_state['redirect'] = 'admin/tripal/extension/brapi/configuration';
}
