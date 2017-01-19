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
    );
    $form['example_settings']['markup'] = array(
      '#markup' => t('Here you can specify example values for the parameters of some BrAPI call. Those values will be as example when the user will try a call on the "Breeding API Overview" public page.<br/><br/>'),
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

  // Storage type selection for some specific fields.
  $storage_options = variable_get('brapi_storage_options');
  if (!$storage_options) {
    $storage_options = array();
  }
  $form['storage_options'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('Storage options'),
    '#collapsible' => TRUE,
    '#collapsed'   => TRUE,
  );
  // Common crop name storage.
  $crop_name_options = array(
    'commonCropNameProp' => t('Stored in stockprop table'),
    'commonCropNameOrganism' => t('Stored in organism table'),
  );
  $crop_name_storage = $storage_options['common_crop_name'];
  if (!$crop_name_storage
      || (($crop_name_storage != 'commonCropNameProp')
          && ($crop_name_storage != 'commonCropNameOrganism'))) {
    $crop_name_storage = 'commonCropNameProp';
  }
  $form['storage_options']['common_crop_name'] = array(
      '#type' => 'select',
      '#description' => t('Select the way common crop name are stored in your database.'),
      '#options' => $crop_name_options, 
      '#default_value' => $crop_name_storage,
      '#title' => t('Common crop name storage'),
      '#title_display' => 'before',
      '#required' => TRUE,
      '#multiple' => FALSE,
  );
  // Genus storage.
  $genus_options = array(
    'genusProp' => t('Stored in stockprop table'),
    'genusOrganism' => t('Stored in organism table'),
  );
  $genus_storage = $storage_options['genus'];
  if (!$genus_storage
      || (($genus_storage != 'genusProp')
          && ($genus_storage != 'genusOrganism'))) {
    $genus_storage = 'genusProp';
  }
  $form['storage_options']['genus_storage'] = array(
      '#type' => 'select',
      '#description' => t('Select the way genus are stored in your database.'),
      '#options' => $genus_options, 
      '#default_value' => $genus_storage,
      '#title' => t('Genus storage'),
      '#title_display' => 'before',
      '#required' => TRUE,
      '#multiple' => FALSE,
  );
  // Species storage.
  $species_options = array(
    'speciesProp' => t('Stored in stockprop table'),
    'speciesOrganism' => t('Stored in organism table'),
  );
  $species_storage = $storage_options['species'];
  if (!$species_storage
      || (($species_storage != 'speciesProp')
          && ($species_storage != 'speciesOrganism'))) {
    $species_storage = 'speciesProp';
  }
  $form['storage_options']['species_storage'] = array(
      '#type' => 'select',
      '#description' => t('Select the way species names are stored in your database.'),
      '#options' => $species_options, 
      '#default_value' => $species_storage,
      '#title' => t('Species storage'),
      '#title_display' => 'before',
      '#required' => TRUE,
      '#multiple' => FALSE,
  );
  // Country of origin code storage.
  $country_of_origin_code_options = array(
    'countryOfOriginCodeProp' => t('Stored in stockprop table'),
    'countryOfOriginCodeCVTerm' => t('Stored in stock_cvterm table'),
  );
  $country_of_origin_code_storage = $storage_options['country_of_origin_code'];
  if (!$country_of_origin_code_storage
      || (($country_of_origin_code_storage != 'countryOfOriginCodeProp')
          && ($country_of_origin_code_storage != 'countryOfOriginCodeCVTerm'))) {
    $country_of_origin_code_storage = 'countryOfOriginCodeProp';
  }
  $form['storage_options']['country_of_origin_code'] = array(
      '#type' => 'select',
      '#description' => t('Select the way country of origin codes are stored in your database.'),
      '#options' => $country_of_origin_code_options, 
      '#default_value' => $country_of_origin_code_storage,
      '#title' => t('Country of origin code storage'),
      '#title_display' => 'before',
      '#required' => TRUE,
      '#multiple' => FALSE,
  );
  // Institute code storage.
  $institute_code_options = array(
    'instituteCodeProp' => t('Stored in stockprop table'),
    'instituteCodeStockCollection' => t('Stored in stockcollection table'),
  );
  $institute_code_storage = $storage_options['institute_code'];
  if (!$institute_code_storage
      || (($institute_code_storage != 'instituteCodeProp')
          && ($institute_code_storage != 'instituteCodeStockCollection'))) {
    $institute_code_storage = 'instituteCodeProp';
  }
  $form['storage_options']['institute_code'] = array(
      '#type' => 'select',
      '#description' => t('Select the way institute codes are stored in your database.'),
      '#options' => $institute_code_options, 
      '#default_value' => $institute_code_storage,
      '#title' => t('Institute code storage'),
      '#title_display' => 'before',
      '#required' => TRUE,
      '#multiple' => FALSE,
  );
  // Institute name storage.
  $institute_name_options = array(
    'instituteNameProp' => t('Stored in stockprop table'),
    'instituteNameStockCollection' => t('Stored in stockcollection table'),
  );
  $institute_name_storage = $storage_options['institute_name'];
  if (!$institute_name_storage
      || (($institute_name_storage != 'instituteNameProp')
          && ($institute_name_storage != 'instituteNameStockCollection'))) {
    $institute_name_storage = 'instituteNameProp';
  }
  $form['storage_options']['institute_name'] = array(
      '#type' => 'select',
      '#description' => t('Select the way institute names are stored in your database.'),
      '#options' => $institute_name_options, 
      '#default_value' => $institute_name_storage,
      '#title' => t('Institute name storage'),
      '#title_display' => 'before',
      '#required' => TRUE,
      '#multiple' => FALSE,
  );

  // Date settings.
  $date_options = array(
    'xsd' => t('YYYY-MM-DD (xsd)'),
    'timestamp' => t('Timestamp (Unix)'),
    'custom' => t('Custom format (specify below)'),
  );
  $default_date_format = variable_get('brapi_date_format');
  if (!$default_date_format
      || (($default_date_format != 'timestamp')
          && ($default_date_format != 'custom')
          && ($default_date_format != 'xsd'))) {
    $default_date_format = 'xsd';
  }
  $form['storage_options']['date_format'] = array(
      '#type' => 'select',
      '#id' => 'brapi_date_settings',
      '#description' => t('Select the format used to store dates in the database.'),
      '#options' => $date_options, 
      '#default_value' => $default_date_format,
      '#title' => t('Date storage format'),
      '#title_display' => 'before',
      '#required' => TRUE,
      '#multiple' => FALSE,
  );
  $form['storage_options']['custom_date_format'] = array(
    '#type' => 'textfield',
    '#id' => 'brapi_custom_date_format',
    '#description' => t('If you use a custom format, specify the way dates are stored in database using the format describbed at <a href="http://php.net/manual/en/datetime.createfromformat.php">DateTime::createFromFormat PHP documentation</a>.'),
    '#default_value' => variable_get('brapi_custom_date_format'),
    '#title' => t('Custom date format specification'),
    '#title_display' => 'before',
    '#size' => 80,
  );

  // CV settings.
  $form['cv_settings'] = array(
    '#type'        => 'fieldset',
    '#attributes'  => array('class' => array('container-inline')),
    '#title'       => t('Controlled vocabulary settings'),
    '#collapsible' => TRUE,
    '#collapsed'   => FALSE,
  );

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
        . ' (cv:' . $term->cv_id->name . '; id:' . $term->cvterm_id . ')';
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
      '#maxlength' => 1024,
      '#field_suffix' => '<br/>&nbsp;&nbsp;&nbsp;',
      '#suffix' => '<br/><br/>',
    );
  }

  // MCPD settings.
  $form['chado_mcpd'] = array(
    '#type'        => 'fieldset',
    '#attributes'  => array('class' => array('container-inline')),
    '#title'       => t('MCPD settings'),
    '#collapsible' => TRUE,
    '#collapsed'   => TRUE,
  );
  
  $form['chado_mcpd']['markup'] = array(
    '#markup' => t('Usually, the MCPD settings should not be changed as they correspond to adopted standards. However, in some cases, you may have used other terms than the recommanded ones so you may need to override MCPD default terms here.<br/><br/>'),
  );

  // MCPD terms override.
  $mcpd_cv_select = array('name' => BRAPI_MULTICROP_PASSPORT_ONTOLOGY_CV);
  $mcpd_cv = chado_select_record('cv', array('*'), $mcpd_cv_select);
  if (is_array($mcpd_cv)) {
    $mcpd_cv = current($mcpd_cv);
  }

  foreach (brapi_get_mcpd_mapping() as $term_name => $term_mapping) {
    $default_ids = array();

    // Get from settings.
    if (array_key_exists($term_name, $brapi_cv_settings)) {
      $default_ids = $brapi_cv_settings[$term_name];
      if ($default_ids) {
        $default_ids = array_map('trim', explode(',', $default_ids));
      }
      else {
        $default_ids = array();
      }
    }

    // Get term from MCPD CV.
    if (empty($default_ids)) {
      $default_ids = brapi_get_cvterm_id($term_mapping);
    }

    if (!empty($default_ids)) {
      $terms = chado_generate_var(
        'cvterm',
        array('cvterm_id' => $default_ids),
        array('return_array' => 1)
      );
      // Turn numeric ids into human-readable values.
      $default_ids = array();
      foreach ($terms as $term) {
        $default_ids[] =
          $term->name
          . ' (cv:' . $term->cv_id->name . '; id:' . $term->cvterm_id . ')';
      }
    }

    if (array_key_exists('name', $term_mapping)) {
      $description = t(
        '<b>"@mcpd_term"</b> is the MCPD term that should be used for this field.',
        array('@mcpd_term' => $term_mapping['name'])
      );
    }
    elseif (array_key_exists('related', $term_mapping)) {
      if (is_array($term_mapping['related'])) {
        $description = t(
          'The MCPD terms that should be used for this field are the <b>children of the term "@mcpd_term"</b>.',
          array('@mcpd_term' => $term_mapping['related']['name'])
        );
      }
      else {
        $description = t(
          'The MCPD terms that should be used for this field are the <b>children of the term "@mcpd_term"</b>.',
          array('@mcpd_term' => $term_mapping['related'])
        );
      }
    }
    else {
      $description = t('No specific MCPD term is recommanded for this field.');
    }

    if (array_key_exists('fallback', $term_mapping)) {
      $description .= ' ' . format_plural(
        count($term_mapping['fallback']),
        'However, the following term could be used as well: @mcpd_term.',
        'However, the following terms could be used as well: @mcpd_term.',
        array('@mcpd_term' => '"' . implode('", "', $term_mapping['fallback']) . '"')
      );
    }

    $form['chado_mcpd'][$term_name] = array(
      '#type' => 'textfield',
      '#description' => $description,
      '#default_value' => implode(', ', $default_ids),
      '#autocomplete_path' => 'brapi/terms/autocomplete',
      '#title' => $term_name,
      '#title_display' => 'before',
      '#required' => TRUE,
      '#size' => 80,
      '#maxlength' => 1024,
      '#field_suffix' => '<br/>&nbsp;&nbsp;&nbsp;',
      '#suffix' => '<br/><br/>',
    );
  }

  // MCPD reload.
  $form['chado_mcpd']['reset_mcpd_desc'] = array(
    '#markup' => t('If your current version of MCPD is not the one supported by this installation of the Breeding API (for instance, terms are missing), you may reload the ontology in Chado using the following button.<br/>'),
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

  // Save storage options.
  $storage_options = variable_get('brapi_storage_options', array());
  if (!is_array($storage_options)) {
    $storage_options = array();
  }
  // Common crop name storage.
  if ($form_state['values']['common_crop_name']) {
    $storage_options['common_crop_name'] = $form_state['values']['common_crop_name'];
  }
  // Genus storage.
  if ($form_state['values']['genus_storage']) {
    $storage_options['genus'] = $form_state['values']['genus_storage'];
  }
  // Species storage.
  if ($form_state['values']['species_storage']) {
    $storage_options['species'] = $form_state['values']['species_storage'];
  }
  // Country of origin storage.
  if ($form_state['values']['country_of_origin_code']) {
    $storage_options['country_of_origin_code'] = $form_state['values']['country_of_origin_code'];
  }
  // Institute code storage.
  if ($form_state['values']['institute_code']) {
    $storage_options['institute_code'] = $form_state['values']['institute_code'];
  }
  // Institute name storage.
  if ($form_state['values']['institute_name']) {
    $storage_options['institute_name'] = $form_state['values']['institute_name'];
  }
  variable_set('brapi_storage_options', $storage_options);

  // Save date format.
  switch ($form_state['values']['date_format']) {
    case 'xsd':
      variable_set('brapi_date_format', 'xsd');
      break;
    case 'timestamp':
      variable_set('brapi_date_format', 'timestamp');
      break;
    case 'custom':
      if ($form_state['values']['custom_date_format']) {
        variable_set('brapi_date_format', 'custom');
        variable_set('brapi_custom_date_format', $form_state['values']['custom_date_format']);
      }
      else {
        drupal_set_message('No custom date format specified while this format has been selected!', 'warning');
      }
      break;
    default:
      drupal_set_message('Invalid date format specification!', 'error');
      break;
  }

  // CV settings.
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
  // MCPD part.
  foreach (brapi_get_mcpd_settings() as $term_label => $term_mapping) {
    $user_terms = $form_state['values'][$term_label];
    $cvterm_ids = array();
    foreach (array_map('trim', explode(',', $user_terms)) as $user_term) {
      if ($user_term) {
        $cvterm_id = brapi_get_cvterm_id($user_term);
        if (!$cvterm_id) {
          $cvterm_id = brapi_get_cvterm_id($user_term, FALSE);
        }
        if ($cvterm_id) {
          $cvterm_ids[$cvterm_id] = $cvterm_id;
        }
        else {
          drupal_set_message('Term not found: "' . $term_label . '"', 'warning');
        }
      }
    }
    
    if (!empty($cvterm_ids)) {
      if (1 == count($cvterm_ids)){
        $brapi_cv_settings[$term_label] = array_values($cvterm_ids)[0];
      }
      else {
        $brapi_cv_settings[$term_label] = array_values($cvterm_ids);
      }
    }
  }
  // Save CV settings.
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
