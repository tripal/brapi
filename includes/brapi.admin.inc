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
function brapi_admin_form($form, &$form_state, $no_js_use = FALSE) {
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
      '#collapsed'   => TRUE,
    );
    $form['example_settings']['markup'] = array(
      '#markup' => t('Here you can specify example values for the parameters of some BrAPI call. Those values will be as example when the user will try a call on the "Breeding API Overview" public page.<br/><br/>'),
    );
    ksort($example_parameters);
  }

  $brapi_example_settings = variable_get(BRAPI_EXAMPLE_SETTINGS, array());

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
  $storage_options = variable_get(BRAPI_STORAGE_OPTIONS, array());
  $form['storage_options'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('Storage options'),
    '#collapsible' => TRUE,
    '#collapsed'   => TRUE,
  );
  // BrAPI DbId.
  $dbid_options = array(
    'uniquename' => t('Use Chado uniquename column'),
    'chado_id' => t('Use Chado table "_id" column'),
  );
  $dbid_storage = isset($storage_options['dbid']) ?
    $storage_options['dbid']
    : '';
  if (!$dbid_storage
      || (($dbid_storage != 'uniquename')
          && ($dbid_storage != 'chado_id'))) {
    $dbid_storage = 'uniquename';
  }
  $form['storage_options']['dbid'] = array(
    '#type' => 'select',
    '#description' => t('Select the column to use for BrAPI *DbId fields (when associated to a Chado table).'),
    '#options' => $dbid_options,
    '#default_value' => $dbid_storage,
    '#title' => t('BrAPI *DbId storage'),
    '#title_display' => 'before',
    '#required' => TRUE,
    '#multiple' => FALSE,
  );
  // Germplasm PUI.
  $germplasm_pui_options = array(
    'germplasmPUI' => t('Stored in stockprop table'),
    'germplasmPUIDrupalURL' => t('Use Drupal stock URL'),
  );
  $germplasm_pui_storage = isset($storage_options['germplasm_pui']) ?
    $storage_options['germplasm_pui']
    : '';
  if (!$germplasm_pui_storage
      || (($germplasm_pui_storage != 'germplasmPUI')
          && ($germplasm_pui_storage != 'germplasmPUIDrupalURL'))) {
    $germplasm_pui_storage = 'germplasmPUI';
  }
  $form['storage_options']['germplasm_pui'] = array(
    '#type' => 'select',
    '#description' => t('Select the way germplasm persistent unique identifier are stored in your database.'),
    '#options' => $germplasm_pui_options,
    '#default_value' => $germplasm_pui_storage,
    '#title' => t('Germplasm PUI storage'),
    '#title_display' => 'before',
    '#required' => TRUE,
    '#multiple' => FALSE,
  );
  // Crop list storage. Naming method selection, and manual override list.
  $crop_naming_method_options = array(
    'nameBySpecies' => t('Use species as crop name'),
    'nameByAbbreviation' => t('Use organism abbreviation as crop name'),
    'nameByCommonName' => t('Use organism common name as crop name'),
    'nameByTaxon' => t('Use full taxon as crop name'),
  );
  $crop_naming_method = isset($storage_options['crop_naming_method']) ?
    $storage_options['crop_naming_method']
    : '';
  if (!$crop_naming_method
      || (($crop_naming_method != 'nameBySpecies')
          && ($crop_naming_method != 'nameByAbbreviation')
          && ($crop_naming_method != 'nameByCommonName')
          && ($crop_naming_method != 'nameByTaxon'))) {
    $crop_naming_method = 'nameBySpecies';
  }
  $form['storage_options']['crop_naming_method'] = array(
    '#type' => 'select',
    '#description' => t('Select the way a crop name is generated for each organism. If the selected method returns no value for an organism, the full taxon will then be used.'),
    '#options' => $crop_naming_method_options,
    '#default_value' => $crop_naming_method,
    '#title' => t('Crop naming method'),
    '#title_display' => 'before',
    '#required' => TRUE,
    '#multiple' => FALSE,
  );
  $crop_list_storage = isset($storage_options['crop_list']) ?
    $storage_options['crop_list']
    : '';
  $form['storage_options']['crop_list'] = array(
    '#type' => 'textfield',
    '#description' => t('Enter the list of (no space, comma-separated) supported crops here. If blank, the "Crop naming method" selected above will be used.'),
    '#default_value' => $crop_list_storage,
    '#title' => t('Manual entry of list of supported crops ("crops" call)'),
    '#title_display' => 'before',
    '#required' => FALSE,
    '#size' => 80,
  );
  // Common crop name storage.
  $crop_name_options = array(
    'commonCropNameProp' => t('Stored in stockprop table'),
    'commonCropNameOrganism' => t('Stored in organism table'),
  );
  $crop_name_storage = isset($storage_options['common_crop_name']) ?
    $storage_options['common_crop_name']
    : '';
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
  $genus_storage = isset($storage_options['genus']) ?
    $storage_options['genus']
    : '';
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
    'speciesOrganism' => t('Stored species in organism table, subtaxa in stockprop table'),
    'speciesInfraFunc' => t('Stored both species and subtaxa in organism table'),
    'speciesFunc' => t('Stored in organism + phylonode tables'),
  );
  $species_storage = isset($storage_options['species']) ?
    $storage_options['species']
    : '';
  if (!$species_storage
      || (($species_storage != 'speciesProp')
          && ($species_storage != 'speciesOrganism')
          && ($species_storage != 'speciesInfraFunc')
          && ($species_storage != 'speciesFunc'))) {
    $species_storage = 'speciesOrganism';
  }
  $form['storage_options']['species_storage'] = array(
    '#type' => 'select',
    '#description' => t('Select the way species and subtaxa names and their associated authorities are stored in your database.'),
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
  $country_of_origin_code_storage = isset($storage_options['country_of_origin_code']) ?
    $storage_options['country_of_origin_code']
    : '';
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
  $institute_code_storage = isset($storage_options['institute_code']) ?
    $storage_options['institute_code']
    : '';
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
  $institute_name_storage = isset($storage_options['institute_name']) ?
    $storage_options['institute_name']
    : '';
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
  $default_date_format = variable_get(BRAPI_DATE_FORMAT, '');
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
    '#default_value' => variable_get(BRAPI_CUSTOM_DATE_FORMAT, ''),
    '#title' => t('Custom date format specification'),
    '#title_display' => 'before',
    '#size' => 80,
  );

  // Trait terms settings.
  $trait_cvs = isset($storage_options['traits']['cvs']) ?
    $storage_options['traits']['cvs']
    : '';
  $trait_parent_cvterms = isset($storage_options['traits']['parent_cvterms']) ?
    $storage_options['traits']['parent_cvterms']
    : '';
  $trait_cvterms = isset($storage_options['traits']['cvterms']) ?
    $storage_options['traits']['cvterms']
    : '';
  $form['storage_options']['traits'] = array(
    '#type' => 'fieldset',
    '#title' => t('Trait terms selection'),
    '#collapsible' => FALSE,
    '#collapsed'   => FALSE,
  );
  $form['storage_options']['traits']['markup'] = array(
    '#markup' => t('You can specify either a list of CVs or a list CV term
      identifiers (parent terms or terms) to let BrAPI know which CV terms
      are traits but not both (CV identifiers and terms identifiers are
      mutually exclusive).<br/><br/>'),
  );
  $form['storage_options']['traits']['trait_cvs'] = array(
    '#type' => 'textfield',
    '#title' => t('List of CV(s) holding traits'),
    '#title_display' => 'before',
    '#description' => t(
      'Specify a list of cv.cv_id. If you include CV names in the list, only
      the numeric identifier cv_id (last number for each CV) will be used by
      the system. You can specify multiple CV with comas. All the terms of
      those CV can be used as traits.
    '),
    '#default_value' => $trait_cvs,
    '#required' => FALSE,
    '#size' => 80,
    '#maxlength' => 2048,
    '#autocomplete_path' => 'brapi/cvs/autocomplete',
  );
  $form['storage_options']['traits']['trait_parent_cvterms'] = array(
    '#type' => 'textfield',
    '#title' => t('Trait parent CV term(s)'),
    '#title_display' => 'before',
    '#description' => t('
      Specify a list of CV term identifiers (cvterm.cvterm_id) that are parent
      terms of other terms used as traits (nb. the parent term is not a trait).
      Child terms must be linked to parents through the cvterm_relationship
      table using a "is_a" or "isa" type of relationship.'),
    '#default_value' => $trait_parent_cvterms,
    '#required' => FALSE,
    '#size' => 80,
    '#maxlength' => 2048,
    '#autocomplete_path' => 'brapi/terms/autocomplete',
  );
  $form['storage_options']['traits']['trait_cvterms'] = array(
    '#type' => 'textfield',
    '#title' => t('Trait CV term(s)'),
    '#title_display' => 'before',
    '#description' => t('
      Specify a list of CV term identifiers (cvterm.cvterm_id) that are used as
      traits.
    '),
    '#default_value' => $trait_cvterms,
    '#required' => FALSE,
    '#size' => 80,
    '#maxlength' => 2048,
    '#autocomplete_path' => 'brapi/terms/autocomplete',
  );

  // CV settings.
  $form['cv_settings'] = array(
    '#type'        => 'fieldset',
    '#attributes'  => array('class' => array('container-inline')),
    '#title'       => t('Controlled vocabulary settings'),
    '#collapsible' => TRUE,
    '#collapsed'   => TRUE,
  );

  $brapi_cv_select = array('name' => BRAPI_CV);
  $brapi_cv = chado_select_record('cv', array('*'), $brapi_cv_select);
  if (is_array($brapi_cv)) {
    $brapi_cv = current($brapi_cv);
  }

  $brapi_cv_settings = brapi_get_cv_settings();
  foreach (brapi_get_cv() as $term_name => $definition) {
    $default_ids = array();

    // Get from settings.
    if (array_key_exists($term_name, $brapi_cv_settings)) {
      $default_ids = $brapi_cv_settings[$term_name];
      if ($default_ids) {
        if (!is_array($default_ids)) {
          $default_ids = array_map('trim', explode(',', $default_ids));
        }
      }
      else {
        $default_ids = array();
      }
    }

    // Get term from BrAPI CV.
    if (empty($default_ids)) {
      $default_ids = brapi_get_cvterm_id(
        array(
          'name' => $term_name,
          'cv_id' => $brapi_cv_select,
        )
      );
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

    // Set a default id for missing terms.
    if (empty($default_ids)) {
      $default_ids = array(0);
    }

    $form['cv_settings'][$term_name] = array(
      '#type' => 'textfield',
      '#description' => $definition,
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
        if (!is_array($default_ids)) {
          $default_ids = array_map('trim', explode(',', $default_ids));
        }
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

    // Set a default id for missing terms.
    if (empty($default_ids)) {
      $default_ids = array(0);
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

    $term_machine_name = preg_replace('/\W+/', '_', $term_name);
    $form['chado_mcpd'][$term_machine_name] = array(
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

  // Germplasm attributes.
  $germplasm_attributes = variable_get(
    BRAPI_GERMPLASM_ATTRIBUTES,
    array(
      array(
        'name'     => '',
        'category' => '',
        'cvs'      => '',
        'cvterms'  => '',
      ),
    )
  );
  $germplasm_category_list = variable_get(BRAPI_GERMPLASM_ATTR_CATEGORIES, array());

  $form['germplasm_attributes'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('Germplasm attribute settings'),
    '#collapsible' => TRUE,
    '#collapsed'   => TRUE,
  );

  $form['germplasm_attributes']['markup'] = array(
    '#markup' => t('Germplasm attributes are stored as stock properties
      (stockprop) or stock CV terms (stock_cvterm) (while traits are stored as
      ND experiments). Here, you can define several attribute categories. Each
      category will regroup attributes using specified list of CV terms or all
      the CV terms included in the specified CVs. It means that the matching
      attributes will have either a stockprop.type_id or a
      stock_cvterm.cvterm_id matching the list of selected CV terms.
      You can specify both a list of CVs and a list CV terms identifiers or
      just one of them (but at least one).
      <br/><br/>'),
  );

  if (empty($form_state['category_count'])) {
    $form_state['category_count'] = count($germplasm_attributes);
  }

  $form['germplasm_attributes']['attribute'] = array(
    '#type' => 'container',
    '#prefix' => '<div id="categories-fieldset-wrapper">',
    '#suffix' => '</div>',
    '#tree' => TRUE,
  );

  for ($i = 0; $i < $form_state['category_count']; ++$i) {
    $category_list_name = $germplasm_attributes[$i]['category'];
    $form['germplasm_attributes']['attribute'][$i] = array(
      '#type' => 'fieldset',
      '#title' => t('Attribute settings'),
    );
    $form['germplasm_attributes']['attribute'][$i]['name'] = array(
      '#type' => 'textfield',
      '#title' => t('Attribute name'),
      '#title_display' => 'before',
      '#description' => t('Leave this field empty to remove this attribute settings.'),
      '#default_value' => $germplasm_attributes[$i]['name'],
      '#required' => FALSE,
      '#size' => 80,
      '#maxlength' => 1024,
    );
    $form['germplasm_attributes']['attribute'][$i]['category'] = array(
      '#type' => 'textfield',
      '#title' => t('Category name'),
      '#title_display' => 'before',
      '#default_value' =>
      $category_list_name ?
      $germplasm_category_list[$category_list_name]
      : '',
      '#required' => FALSE,
      '#size' => 80,
      '#maxlength' => 1024,
    );
    $form['germplasm_attributes']['attribute'][$i]['cvs'] = array(
      '#type' => 'textfield',
      '#title' => t('Associated CV(s)'),
      '#title_display' => 'before',
      '#description' => t(
        'Specify a list of cv.cv_id. If you include CV names in the list, only
        the numeric identifier cv_id (last number for each CV) will be used by
        the system. You can specify multiple CV with comas. All the terms of
        those CV will be used to match attributes.
      '),
      '#default_value' => $germplasm_attributes[$i]['cvs'],
      '#required' => FALSE,
      '#size' => 80,
      '#maxlength' => 2048,
      '#autocomplete_path' => 'brapi/cvs/autocomplete',
    );
    $form['germplasm_attributes']['attribute'][$i]['cvterms'] = array(
      '#type' => 'textfield',
      '#title' => t('Associated CV term(s)'),
      '#title_display' => 'before',
      '#description' => t('
        Specify a list of CV term identifiers (cvterm.cvterm_id) that will be
        used to match attributes.
      '),
      '#default_value' => $germplasm_attributes[$i]['cvterms'],
      '#required' => FALSE,
      '#size' => 80,
      '#maxlength' => 2048,
      '#autocomplete_path' => 'brapi/terms/autocomplete',
    );
  }

  $form['germplasm_attributes']['add_category'] = array(
    '#type' => 'submit',
    '#value' => t('Add a category'),
    '#submit' => array('brapi_add_category_add_one'),
    '#ajax' => array(
      'callback' => 'brapi_add_category_callback',
      'wrapper' => 'categories-fieldset-wrapper',
    ),
  );

  // Setup calls options.
  $form['call_options'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('Call options'),
    '#collapsible' => TRUE,
    '#collapsed'   => TRUE,
  );
  // Implementation version.
  $version_options = variable_get(BRAPI_VERSION_OPTIONS, array());
  // Aggregation.
  $aggregation_options = variable_get(BRAPI_AGGREGATION_OPTIONS, array());
  // Get the list of available BrAPI sites.
  $brapi_sites = array();
  $brapi_site_options = array(
    '' => t('None'),
  );
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'brapi_site');
  $results = $query->execute();
  if (!empty($results)) {
    // Load data about each connection.
    $brapi_sites = entity_load('brapi_site', array_keys($results['brapi_site']));

    foreach ($brapi_sites as $bsid => $entity) {
      $brapi_site_options[$entity->machine_name] = $entity->title;
    }
  }

  // Loop on calls.
  foreach (brapi_get_calls() as $call_path => $call) {
    // Skip root call.
    if (!$call_path) {
      continue;
    }
    $call_path_form_name = preg_replace('/\W+/', '_', $call_path);
    $form['call_options']['call_' . $call_path_form_name] = array(
      '#type'        => 'fieldset',
      '#title'       => $call_path,
      '#collapsible' => TRUE,
      '#collapsed'   => FALSE,
    );
    $call_section = &$form['call_options']['call_' . $call_path_form_name];

    $available_versions = array_merge(
      array(
        '0' => t('Not provided'),
        'proxy' => t('Proxied'),
      ),
      array_combine(
        array_keys($call['callback versions']),
        array_keys($call['callback versions'])
      )
    );
    $default_value = '0';
    if (isset($version_options[$call_path])) {
      $default_value = $version_options[$call_path];
    }
    elseif ($call) {
      $default_value = $call['active version'];
    }
    $call_section['version_' . $call_path_form_name] = array(
      '#type' => 'select',
      '#title' => t('Select a call version'),
      '#options' => $available_versions,
      '#default_value' => $default_value,
      '#title_display' => 'before',
      '#required' => TRUE,
      '#multiple' => FALSE,
    );

    if ($call['aggregable']) {
      $default_values =
        (isset($aggregation_options[$call_path])
            && $aggregation_options[$call_path]) ?
          $aggregation_options[$call_path]
          : array('');
      $call_section['aggr_' . $call_path_form_name] = array(
        '#type' => 'select',
        '#options' => $brapi_site_options,
        '#default_value' => $default_values,
        '#title' => t('Proxy for/aggregation with'),
        '#title_display' => 'before',
        '#description' => t('Select BrAPI sites for aggregation. Use Ctrl+click to select/unselect multiple sites.'),
        '#required' => FALSE,
        '#multiple' => TRUE,
      );
    }
  }

  // Save button.
  $form['save_settings'] = array(
    '#type'        => 'submit',
    '#value'       => t('Save configuration'),
    '#submit'      => array('brapi_admin_form_submit'),
  );

  return $form;
}

/**
 * Callback for both ajax-enabled buttons.
 *
 * Selects and returns the fieldset with the names in it.
 */
function brapi_add_category_callback($form, $form_state) {
  return $form['germplasm_attributes']['attribute'];
}

/**
 * Submit handler for the "add-one-more" button.
 *
 * Increments the max counter and causes a rebuild.
 */
function brapi_add_category_add_one($form, &$form_state) {
  $form_state['category_count']++;
  $form_state['rebuild'] = TRUE;
}

/**
 * Validate Breeding API configuration changes.
 *
 * @ingroup brapi
 */
function brapi_admin_form_validate($form_id, &$form_state) {
  if (!empty($form_state['values']['trait_cvs'])
      && (!empty($form_state['values']['trait_parent_cvterms'])
          || !empty($form_state['values']['trait_cvterms']))) {
    form_set_error('traits', t('You can not mix CV identifiers and CV term identifiers for traits. Please only fill CV identifiers OR CV term identifiers (simple terms, parents or both).'));
  }
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
  $storage_options = variable_get(BRAPI_STORAGE_OPTIONS, array());
  if (!is_array($storage_options)) {
    $storage_options = array();
  }
  // BrAPI *DbId storage.
  if ($form_state['values']['dbid']) {
    $storage_options['dbid'] = $form_state['values']['dbid'];
  }
  // Germplasm PUI storage.
  if ($form_state['values']['germplasm_pui']) {
    $storage_options['germplasm_pui'] = $form_state['values']['germplasm_pui'];
  }
  // Crop list storage method and override list.
  if ($form_state['values']['crop_naming_method']) {
    $storage_options['crop_naming_method'] = $form_state['values']['crop_naming_method'];
  }
  if ($form_state['values']['crop_list']) {
    $storage_options['crop_list'] = $form_state['values']['crop_list'];
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

  // Save date format.
  switch ($form_state['values']['date_format']) {
    case 'xsd':
      variable_set(BRAPI_DATE_FORMAT, 'xsd');
      break;

    case 'timestamp':
      variable_set(BRAPI_DATE_FORMAT, 'timestamp');
      break;

    case 'custom':
      if ($form_state['values']['custom_date_format']) {
        variable_set(BRAPI_DATE_FORMAT, 'custom');
        variable_set(BRAPI_CUSTOM_DATE_FORMAT, $form_state['values']['custom_date_format']);
      }
      else {
        drupal_set_message(t('No custom date format specified while this format has been selected!'), 'warning');
      }
      break;

    default:
      drupal_set_message(t('Invalid date format specification!'), 'error');
      break;
  }

  // Save date traits.
  $trait_cvs = $trait_parent_cvterms = $trait_cvterms = '';
  if (!empty($form_state['values']['trait_cvs'])) {
    $trait_cvs = $form_state['values']['trait_cvs'];
  }
  if (!empty($form_state['values']['trait_parent_cvterms'])) {
    $trait_parent_cvterms = $form_state['values']['trait_parent_cvterms'];
  }
  if (!empty($form_state['values']['trait_cvterms'])) {
    $trait_cvterms = $form_state['values']['trait_cvterms'];
  }
  $storage_options['traits'] = array(
    'cvs' => $trait_cvs,
    'parent_cvterms' => $trait_parent_cvterms,
    'cvterms' => $trait_cvterms,
  );

  variable_set(BRAPI_STORAGE_OPTIONS, $storage_options);

  // CV settings.
  $brapi_cv_settings = brapi_get_cv_settings();
  foreach (brapi_get_cv() as $term_label => $definition) {
    $user_terms = $form_state['values'][$term_label];
    $user_term_array = array_filter(array_map(
      function ($a) {
        return trim(str_replace("\\,", ',', $a));
      },
      preg_split('/(?<!\\\\),/', $user_terms)
    ));
    $cvterm_ids = array();
    foreach ($user_term_array as $user_term) {
      $cvterm_id = brapi_get_cvterm_id($user_term, FALSE);
      if ($cvterm_id) {
        $cvterm_ids[$cvterm_id] = $cvterm_id;
      }
      else {
        drupal_set_message(t('Term not found: "@term_label"', array('@term_label' => $term_label)), 'warning');
      }
    }

    if (empty($cvterm_ids)) {
      $brapi_cv_settings[$term_label] = 0;
    }
    elseif (1 == count($cvterm_ids)) {
      $brapi_cv_settings[$term_label] = array_values($cvterm_ids)[0];
    }
    else {
      $brapi_cv_settings[$term_label] = array_values($cvterm_ids);
    }
  }

  // MCPD part.
  foreach (brapi_get_mcpd_settings() as $term_label => $term_mapping) {
    $term_machine_name = preg_replace('/\W+/', '_', $term_label);
    $user_terms = $form_state['values'][$term_machine_name];
    $user_term_array = array_filter(array_map(
      function ($a) {
        return trim(str_replace("\\,", ',', $a));
      },
      preg_split('/(?<!\\\\),/', $user_terms)
    ));
    $cvterm_ids = array();
    foreach ($user_term_array as $user_term) {
      if ($user_term) {
        $cvterm_id = brapi_get_cvterm_id($user_term);
        if (!$cvterm_id) {
          $cvterm_id = brapi_get_cvterm_id($user_term, FALSE);
        }
        if ($cvterm_id) {
          $cvterm_ids[$cvterm_id] = $cvterm_id;
        }
        else {
          drupal_set_message(t('Term not found: "@term_label"', array('@term_label' => $term_label)), 'warning');
        }
      }
    }

    if (empty($cvterm_ids)) {
      $brapi_cv_settings[$term_label] = 0;
    }
    elseif (1 == count($cvterm_ids)) {
      $brapi_cv_settings[$term_label] = array_values($cvterm_ids)[0];
    }
    else {
      $brapi_cv_settings[$term_label] = array_values($cvterm_ids);
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

  // Example options.
  $brapi_example_settings = array();
  foreach ($example_parameters as $example_parameter) {
    $brapi_example_settings[$example_parameter['name']] = $form_state['values'][$example_parameter['name']];
  }
  variable_set(BRAPI_EXAMPLE_SETTINGS, $brapi_example_settings);

  // Save germplasm attributes.
  $germplasm_attributes = array();
  $germplasm_category_list = variable_get(BRAPI_GERMPLASM_ATTR_CATEGORIES, array());
  $germplasm_categories = array_flip($germplasm_category_list);
  if (!empty($form_state['values']['attribute'])) {
    foreach ($form_state['values']['attribute'] as $attribute) {
      if ($attribute['name']
          && ($attribute['cvs'] || $attribute['cvterms'])) {
        // Replace category name by its index.
        if (array_key_exists($attribute['category'], $germplasm_categories)) {
          // Get existing index.
          $attribute['category'] = $germplasm_categories[$attribute['category']];
        }
        else {
          // Category does not exist, add it.
          $germplasm_category_list[] = $attribute['category'];
          // Place array index on last inserted key.
          end($germplasm_category_list);
          // Save index.
          $attribute['category'] =
            $germplasm_categories[$attribute['category']] =
            key($germplasm_category_list);
        }
        $germplasm_attributes[] = $attribute;
      }

    }
  }
  variable_set(BRAPI_GERMPLASM_ATTRIBUTES, $germplasm_attributes);
  variable_set(BRAPI_GERMPLASM_ATTR_CATEGORIES, $germplasm_category_list);

  // Version options.
  $version_options = variable_get(BRAPI_VERSION_OPTIONS, array());
  // Aggregation options.
  $brapi_aggregation_options = variable_get(BRAPI_AGGREGATION_OPTIONS, array());

  // Loop on calls.
  foreach (brapi_get_calls() as $call_path => $call) {
    // Skip root call.
    if (!$call_path) {
      continue;
    }
    $call_path_form_name = preg_replace('/\W+/', '_', $call_path);
    $version_options[$call_path] = $form_state['values']['version_' . $call_path_form_name];
    if ($call['aggregable']) {
      $brapi_aggregation_options[$call_path] = array_keys($form_state['values']['aggr_' . $call_path_form_name]);
    }
  }
  variable_set(BRAPI_VERSION_OPTIONS, $version_options);
  variable_set(BRAPI_AGGREGATION_OPTIONS, $brapi_aggregation_options);

  drupal_set_message(t('BrAPI configuration saved.'));
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
  $form = array('confirm' => array('#type' => 'hidden', '#value' => 'true'));

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
    // For using brapi_load_obo().
    module_load_install('brapi');
    $mcpd_path = drupal_get_path('module', 'brapi') . '/cv/mcpd_v2.1_151215.obo';
    $loaded = brapi_load_obo('Multi-Crop Passport Descriptors Ontology', $mcpd_path);
    if ($loaded) {
      drupal_set_message(t('MCPD CV loaded or updated successfully.'));
    }
    else {
      drupal_set_message(t('Failed to load or update MCPD CV!'), 'warning');
    }
  }

  $form_state['redirect'] = 'admin/tripal/extension/brapi/configuration';
}
