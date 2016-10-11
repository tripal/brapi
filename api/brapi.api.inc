<?php

/**
 * @file
 * Provides an application programming interface (API) for working with breeding
 * data and breeding API module.
 *
 * @ingroup brapi
 */

/**
 * @defgroup brapi_api API of Breeding API module
 * @ingroup brapi
 * @{
 * Provides an application programming interface (API) for working with breeding
 * data and breeding API module.
 * @}
 */
 
/**
 * Returns multi-crop passport descriptor controlled vocabulary settings used
 * by this Breeding API installation.
 *
 * @return array
 *   keys are MCPD terms and values are arrays of associated chado cvterm_id.
 *
 * @ingroup brapi_api
 */
function brapi_get_mcpd_settings() { 
  return array(
    'BiologicalStatusOfAccessionCode' =>
      brapi_get_children_cvterm_id('BiologicalStatusOfAccessionCode'),

    'TypeOfGermplasmStorageCode'      =>
      brapi_get_children_cvterm_id('TypeOfGermplasmStorageCode'),
  );
}

/**
 * Returns an array containing the CV settings used by this Breeding API
 * installation.
 *
 * @param boolean $reset
 *   Clear current settings and reload them from database.
 *
 * @return array
 *   key are Breeding API term/field names and values are corresponding Chado
 *   cvterm_id.
 *
 * @ingroup brapi_api
 */
function brapi_get_cv_settings($reset = FALSE) { 
  static $settings;
  // if not initialized, get it from cache if available
  if (!isset($settings) || $reset) {
    if (!$reset
        && ($cache = cache_get(BRAPI_CV_SETTINGS))
        && !empty($cache->data)) {
      $settings = unserialize($cache->data);
    }
    else {
      // not available in cache, get it from saved settings
      $settings = variable_get(BRAPI_CV_SETTINGS);
      if (!$settings) {
        // not available in settings, gets default
        $brapi_cv_select = array('name' => BRAPI_CV);
        $brapi_cv = chado_select_record('cv', array('cv_id'), $brapi_cv_select);
        if (is_array($brapi_cv)) {
          $brapi_cv = current($brapi_cv);
        }

        if (!$brapi_cv) {
          drupal_set_message(t('BrAPI controlled vocabulary is missing! Re-install BrAPI module.'), 'error');
        }
  
        $brapi_cv_settings = array();
        foreach (array_keys(brapi_get_cv()) as $term_name) {
          // get term from BrAPI CV
          $term = tripal_get_cvterm(
                    array(
                      'name' => $term_name,
                      'cv_id' => $brapi_cv->cv_id,
                    )
          );
          if ($term) {
            $brapi_cv_settings[$term_name] = $term->cvterm_id;
          }
          else {
            $brapi_cv_settings[$term_name] = 0;
            drupal_set_message(t('BrAPI term @term_name not found in BrAPI controlled vocabulary! You may need to re-install BrAPI module.', array('@term_name' => $term_name)), 'warning');
          }
        }

        // merge with MCPD terms
        $mcpd_cv_settings = brapi_get_mcpd_settings();
        $settings = array_merge($brapi_cv_settings, $mcpd_cv_settings);
      }
      elseif ($reset) {
        // merge with MCPD terms
        $mcpd_cv_settings = brapi_get_mcpd_settings();
        $settings = array_merge($settings, $mcpd_cv_settings);
      }

      cache_set(BRAPI_CV_SETTINGS, 'cache', serialize($settings));
    }
  }
  return $settings;
}


/**
 * Autocomplete helper function that returns suggestions of Chado CV terms
 * according to user inputs.
 *
 * @param string $string
 *   part of the CV term name to use for the CV term search
 *
 * @return array
 *   keys are suggested CV term names with CV name and cvterm_id as text
 *   (computer-processable) and values are corresponding CV term names with CV
 *   (human-readable).
 *
 * @ingroup brapi_api
 */
function brapi_terms_autocomplete($string) {
  $matches = array();
  
  $sql_query = "
      SELECT
        cvt.cvterm_id AS \"id\",
        cvt.name AS \"term\",
        cv.name AS \"cv\"
      FROM
        cvterm cvt
          INNER JOIN cv USING (cv_id)
      WHERE
        cvt.name ILIKE :user_input
  ;";
  $results = chado_query($sql_query, array(':user_input' => $string . '%'));

  // save the query to matches
  foreach ($results as $row) {
    $value = $row->term . ' (cv:' . $row->cv . ', id:' . $row->id . ')';
    $text = $row->term . ' (from ' . $row->cv . ' CV)';
    $matches[$value] = check_plain($text);
  }

  // Return the result to the form in json
  drupal_json_output($matches);
}


/**
 * Helper function that returns the value of a given property for a given stock.
 *
 * @param integer $stock_id
 *   The Chado stock_id value.
 *
 * @param string $property_name
 *   Name of the CV term corresponding to the property to fetch.
 *
 * @param string $cv_name
 *   Name of the CV the property term belongs to.
 *   Default: BRAPI_MULTICROP_PASSPORT_ONTOLOGY_CV
 *
 * @return string
 *   the property value.
 *
 * @ingroup brapi_api
 */
function brapi_get_stock_property_values($stock_id, $property_name, $cv_name = BRAPI_MULTICROP_PASSPORT_ONTOLOGY_CV) {
  $properties = chado_get_property(
    array(
      'table' => 'stock',
      'id'    => $stock_id,
    ),
    array(
      'type_name' => $property_name,
      'cv_name'   => $cv_name,
    )
  );

  $values = array();
  // sort by rank
  if (is_array($properties)) {
    foreach ($properties as $property) {
      $values[$property->rank] = $property->value;
    }
  }
  elseif (!empty($properties)) {
    $values[$properties->rank] = $properties->value;
  }

  return $values;
}


/**
 * Helper function that returns an array of CV terms associated to the given
 * stock.
 *
 * @param integer $stock_id
 *    The Chado stock_id value.
 *
 * @param array $term_selectors
 *   A hash containing any combination of the following keys:
 *   - 'cv' (string/array of string): one or more CHADO cv names
 *   - 'cv_id' (integer/array of integer): one or more CHADO cv_id
 *   - 'term_id' (integer/array of integer): one or more CHADO cvterm_id
 *   - 'term' (string): a single CHADO cvterm name
 *   - 'terms' (array of string): one or more CHADO cvterm names
 *   - 'parent_term_id' (integer/array of integer): CHADO cvterm_id of one or
 *     more parent terms
 *   - 'parent_term' (string): CHADO cvterm name of the parent cvterm
 *   - 'parent_terms' (array of string): array of CHADO cvterm names of parents
 *     cvterms
 *
 * @return array
 *   an array of Tripal CHADO CV term objects or an empty array if no matching
 *   term was found.
 *
 * @ingroup brapi_api
 */
function brapi_get_stock_cvterm_values($stock_id, $term_selectors) {

  $no_constraint = true;
  // check if we have a limited list of CV to look into
  $cv_ids = array();
  if (!empty($term_selectors['cv'])) {
    if (is_array($term_selectors['cv'])) {
      foreach ($term_selectors['cv'] as $cv_name) {
          $cv_ids[] = tripal_get_cv(array('name' => $cv_name))->cv_id;
      }
    }
    else {
      $cv_ids[] = tripal_get_cv(array('name' => $term_selectors['cv']))->cv_id;
    }
  }

  if (!empty($term_selectors['cv_id'])) {
    if (is_array($term_selectors['cv_id'])) {
      $cv_ids = array_merge($cv_ids, $term_selectors['cv_id']);
    }
    else {
      $cv_ids[] = $term_selectors['cv_id'];
    }
  }

  // check if we got a list of term ids
  $term_ids = array();
  if (!empty($term_selectors['term_id'])) {
    $no_constraint = false;
    if (is_array($term_selectors['term_id'])) {
      $term_ids = array_merge($term_ids, $term_selectors['term_id']);
    }
    else {
      $term_ids[] = $term_selectors['term_id'];
    }
  }

  // check if we got a list of term names
  if (!empty($term_selectors['term']) || !empty($term_selectors['terms'])) {

    $no_constraint = false;
    // only work on one list of terms
    if (empty($term_selectors['terms'])) {
      $term_selectors['terms'] = array();
    }
    if (!empty($term_selectors['term'])) {
      $term_selectors['terms'][] = $term_selectors['term'];
    }

    // get term IDs
    foreach ($term_selectors['terms'] as $term_name) {
      if (count($cv_ids)) {
        foreach ($cv_ids as $cv_id) {
          $term = tripal_get_cvterm(
            array(
              'name' => $term_name,
              'cv_id' => $cv_id,
            )
          );
          if ($term) {
            $term_ids[] = $term->cvterm_id;
          }
        }
      }
      else {
        $term = tripal_get_cvterm(array('name' => $term_name,));
        if ($term) {
          $term_ids[] = $term->cvterm_id;
        }
      }
    }

    // make sure we restrict on cvterm_id even when no macth were found
    if (!count($term_ids)) {
      $term_ids = array(0);
    }
  }


  // check if we got a parent terms
  $parent_term_ids = array();
  if (!empty($term_selectors['parent_term_id'])) {
    $no_constraint = false;
    if (is_array($term_selectors['parent_term_id'])) {
      $parent_term_ids = array_merge(
        $parent_term_ids,
        $term_selectors['parent_term_id']
      );
    }
    else {
      $parent_term_ids[] = $term_selectors['parent_term_id'];
    }
  }

  // only work on one list
  if (empty($term_selectors['parent_terms'])) {
    $term_selectors['parent_terms'] = array();
  }
  if (!empty($term_selectors['parent_term'])) {
    $term_selectors['parent_terms'][] = $term_selectors['parent_term'];
  }

  // get parent term IDs from names
  foreach ($term_selectors['parent_terms'] as $parent_term_name) {
    $no_constraint = false;
    if (count($cv_ids)) {
      foreach ($cv_ids as $cv_id) {
        $term = tripal_get_cvterm(
          array(
            'name' => $parent_term_name,
            'cv_id' => $cv_id,
          )
        );
        if ($term) {
          $parent_term_ids[] = $term->cvterm_id;
        }
      }
    }
    else {
      $term = tripal_get_cvterm(array('name' => $parent_term_name,));
      if ($term) {
        $parent_term_ids[] = $term->cvterm_id;
      }
    }
  }

  // now get descendant terms
  if (count($parent_term_ids)) {
    $columns = array('subject_id');
    $values =  array(
      'object_id' => $parent_term_ids,
      'type_id' => array (
          'cv_id' => array (
             'name' => 'relationship',
          ),
          'name' => 'is_a',
          'is_obsolete' => 0,
       ),
    );

    $cvterm_relationship = chado_select_record(
      'cvterm_relationship',
      $columns,
      $values
    );
    foreach ($cvterm_relationship as $result) {
      $term_ids[] = $result->subject_id;
    }
  }

  // get the list of selected terms associated to the given accession
  $columns = array('cvterm_id');
  $values =  array(
    'stock_id' => $stock_id,
  );

  // check if just a CV ID has been specified
  if ($no_constraint && count($cv_ids)) {
    // get all terms from the given CV
    $values['cvterm_id'] = array('cv_id' => $cv_ids);
  }
  elseif (count($term_ids)) {
    $values['cvterm_id'] = $term_ids;
  }

  $terms = array();
  if ($stock_cvterms = chado_select_record('stock_cvterm', $columns, $values)) {
    if (is_array($stock_cvterms)) {
      foreach ($stock_cvterms as $stock_cvterm) {
        $terms[] = tripal_get_cvterm(
          array(
            'cvterm_id' => $stock_cvterm->cvterm_id,
          )
        );
      }
    }
    else {
      $terms[] = tripal_get_cvterm(
        array(
          'cvterm_id' => $stock_cvterms->cvterm_id,
        )
      );
    }
  }

  return $terms;
}

/**
 * Given a parent CV term name and an optional CV name, it returns a sorted
 * array of cvterm_id corresponding to the children CV terms.
 *
 * @param string $parent_cvterm
 *   name of the parent CV term.
 *
 * @param string $cv
 *   name of the CV.
 *
 * @return array
 *   returns an array of sorted cvterm_id
 *
 * @ingroup brapi_api
 */
function brapi_get_children_cvterm_id($parent_cvterm, $cv = BRAPI_MULTICROP_PASSPORT_ONTOLOGY_CV) {
  // get parent term IDs from names
  $term = tripal_get_cvterm(
    array(
      'name' => $parent_cvterm,
      'cv_id' => array(
        'name' => $cv,
      ),
    )
  );

  // now get descendant terms
  $columns = array('subject_id');
  $values =  array(
    'object_id' => $term->cvterm_id,
    'type_id' => array (
        'cv_id' => array (
           'name' => 'relationship',
        ),
        'name' => 'is_a',
        'is_obsolete' => 0,
     ),
  );

  $term_ids = array();
  $cvterm_relationship = chado_select_record(
    'cvterm_relationship',
    $columns,
    $values
  );
  foreach ($cvterm_relationship as $result) {
    $term_ids[] = $result->subject_id;
  }

  sort($term_ids);

  return $term_ids;
}

/**
 * Returns a breeding API field value according to a given data type, a given
 * loaded object and a given field name. Supported fields are defined in the
 * Breeding API module data mapping.
 *
 * @param string $data_type
 *   name of the data type. See keys of the array returned by
 *   brapi_get_data_mapping().
 *
 * @param object $object
 *   a Tripal object like a Tripal stock for instance.
 *
 * @param string $field_name
 *   a field name such as the one returned as keys of the field 'fields' of a
 *   datatype returned by brapi_get_data_mapping().
 *
 * @see brapi_get_data_mapping()
 *
 * @ingroup brapi_api
 */
function brapi_get_field($data_type, $object, $field_name) {
  $data_mapping = brapi_get_data_mapping();
  // make sure we got a mapping defined
  if (!array_key_exists($data_type, $data_mapping)) {
    return brapi_raise_error_json(
      400,
      t(
        "Bad request! Unknown data type '@type'!",
        array('@type' => $data_type)
      )
    );
  }

  if (!array_key_exists($field_name, $data_mapping[$data_type]['fields'])) {
    return brapi_raise_error_json(
      400,
      t(
        "Bad request! Unknown field '@field' for data type '@type'!",
        array('@field' => $field_name, '@type' => $data_type,)
      )
    );
  }
  
  // get values using mapping
  $field_fetcher = $data_mapping[$data_type]['fields'][$field_name];
  $value = '';
  switch (gettype($field_fetcher)) {
    case 'string':
      // get from object
      $value = $object->$field_fetcher;
      break;
    case 'array':
      // get from database using mapping config
      if (isset($data_mapping[$data_type]['fields'][$field_name]['join_key'])) {
        $join_key = $data_mapping[$data_type]['fields'][$field_name]['join_key'];
      }
      else {
        $join_key = $data_mapping[$data_type]['identifier'];
      }
      $records = chado_select_record(
         $data_mapping[$data_type]['fields'][$field_name]['table'],
         array($data_mapping[$data_type]['fields'][$field_name]['column']),
         array_merge(
           array($join_key => $object->$data_mapping[$data_type]['identifier']),
           $data_mapping[$data_type]['fields'][$field_name]['selector']
         )
      );
      // force Tripal return value to be an array for easier management
      if (!is_array($records)) {
        $records = array($records);
      }
      $values = array();
      foreach ($records as $record) {
        $current_value = $record->$data_mapping[$data_type]['fields'][$field_name]['column'];
        // check type and return the appropriate conversion
        switch ($data_mapping[$data_type]['fields'][$field_name]['field_type']) {
          case 'boolean':
            $current_value = $current_value ? TRUE : FALSE;
            break;
          case 'integer':
            $current_value = intval($current_value);
            break;
          case 'string':
            break;
          case 'cvterm':
            if ($term = tripal_get_cvterm(array('cvterm_id' => $current_value,))) {
              $current_value = $term->name;
            }
            else {
              $current_value = NULL;
            }
            break;
          default:
            return brapi_raise_error_json(
              400,
              t(
                "Bad request! Invalid field '@field' configuration for data type '@type' (returns field data type '@field_type')!",
                array(
                  '@field'      => $field_name,
                  '@type'       => $data_type,
                  '@field_type' => $data_mapping[$data_type]['fields'][$field_name]['field_type']
                )
              )
            );
        }
        $values[] = $current_value;
      }
      // check if array requested
      if ($data_mapping[$data_type]['fields'][$field_name]['return_array']) {
        // multiple values
        $value = $values;
      }
      else {
        // a single value
        $value = current($values);
      }
      break;
    case 'object':
      // get from a function
      if ('Closure' == get_class($field_fetcher)) {
        $value = $field_fetcher($data_type, $object, $field_name, 'read');
      }
      break;
    default: 
      return brapi_raise_error_json(
        400,
        t(
          "Bad request! Invalid field '@field' configuration for data type '@type' (field type '@fetcher')!",
          array(
            '@field'    => $field_name,
            '@type'     => $data_type,
            '@fetcher'  => $field_fetcher,
          )
        )
      );
  }
  
  return $value;
}

/**
 * Set the value(s) of a field of a breeding data object. Multiple values can be
 * assigned to a same field (for instance several CV terms to attach to an
 * object). When updating an array of values, if $update is set to TRUE,
 * previous values are removed and only the new ones are set.
 *
 * @param string $data_type
 *   name of the data type. See keys of the array returned by
 *   brapi_get_data_mapping().
 *
 * @param object $object
 *   a Tripal object like a Tripal stock for instance.
 *
 * @param string $field_name
 *   a field name such as the one returned as keys of the field 'fields' of a
 *   datatype returned by brapi_get_data_mapping().
 *
 * @param mixed $field_value
 *   new value or array of values for the given field.
 *
 * @param boolean $update
 *   if FALSE, only tries to insert a new record. If TRUE, updates existing
 *   record by removing previous value(s) and if missing, creates a new one.
 *
 * @see brapi_get_data_mapping()
 *
 * @ingroup brapi_api
 */
function brapi_set_field($data_type, &$object, $field_name, &$field_value, $update=TRUE) {
  $data_mapping = brapi_get_data_mapping();
  // make sure we got a mapping defined
  if (!array_key_exists($data_type, $data_mapping)) {
    return brapi_raise_error_json(
      400,
      t(
        "Bad request! Unknown data type '@type'!",
        array('@type' => $data_type,)
      )
    );
  }
  if (!array_key_exists($field_name, $data_mapping[$data_type]['fields'])) {
    return brapi_raise_error_json(
      400,
      t(
        "Bad request! Unknown field '@field' for data type '@type'!",
        array('@field' => $field_name, '@type' => $data_type,)
      )
    );
  }
  
  // set values using mapping
  $field_fetcher = $data_mapping[$data_type]['fields'][$field_name];
  switch (gettype($field_fetcher)) {
    case "string":
      // set through object
      $object->$field_fetcher = $field_value;
      //+FIXME: $object->save(); ?
      break;
    case "array":
      // set using mapping config
      if (is_array($field_value)) {
        // multiple values
        // check for ranking
        $rank = 0;
        if ($data_mapping[$data_type]['fields'][$field_name]['use_ranking']) {
          // we use ranking
          $ranking = array('rank' => &$rank);
        }
        else {
          $ranking = array();
        }
        if ($update) {
          // remove previous values
          // we capture output buffer for Tripal error messages
          ob_start();
          $deleted_record = chado_delete_record(
            $data_mapping[$data_type]['fields'][$field_name]['table'],
            array_merge(
              array(
                $data_mapping[$data_type]['identifier']
                  => $object->$data_mapping[$data_type]['identifier'],
              ),
              $data_mapping[$data_type]['fields'][$field_name]['selector']
            )
          );
          if (!$deleted_record) {
            $tripal_message = ob_get_clean();
            return brapi_raise_error_json(
              500,
              t(
                "Unable to remove previous field values before update!\n@tripal_message",
                array('@tripal_message' => $tripal_message,)
              )
            );
          }
          ob_get_clean();
          // previous values removed
        }
        // insert new values
        foreach ($field_value as $single_value) {
          // we capture output buffer for Tripal error messages
          ob_start();
          if (!chado_insert_record(
             $data_mapping[$data_type]['fields'][$field_name]['table'],
             array_merge(
               array(
                 $data_mapping[$data_type]['identifier']
                   => $object->$data_mapping[$data_type]['identifier'],
               ),
               array(
                 $data_mapping[$data_type]['fields'][$field_name]['column']
                   => $single_value,
               ),
               $ranking,
               $data_mapping[$data_type]['fields'][$field_name]['selector']
             )
          )) {
            $tripal_message = ob_get_clean();
            return brapi_raise_error_json(
              400,
              t(
                "Failed to save field '@field'!\n@tripal_message",
                array(
                  '@field'          => $field_name,
                  '@tripal_message' => $tripal_message,
                )
              )
            );
          }
          ob_get_clean();
          ++$rank;
        }
      }
      else {
        // single value
        // we capture output buffer for Tripal error messages
        ob_start();
        $updated_record = FALSE;
        if ($update) {
          $updated_record = chado_update_record(
             $data_mapping[$data_type]['fields'][$field_name]['table'],
             array(
               $data_mapping[$data_type]['identifier']
                 => $object->$data_mapping[$data_type]['identifier'],
             ),
             array_merge(
               array(
                 $data_mapping[$data_type]['fields'][$field_name]['column']
                   => $field_value,
               ),
               $data_mapping[$data_type]['fields'][$field_name]['selector']
             )
          );
        }
        if (!$updated_record && !chado_insert_record(
           $data_mapping[$data_type]['fields'][$field_name]['table'],
           array_merge(
             array(
               $data_mapping[$data_type]['identifier']
                 => $object->$data_mapping[$data_type]['identifier'],
             ),
             array(
               $data_mapping[$data_type]['fields'][$field_name]['column']
                 => $field_value,
             ),
             $data_mapping[$data_type]['fields'][$field_name]['selector']
           )
        )) {
          $tripal_message = ob_get_clean();
          return brapi_raise_error_json(
            400,
            t(
              "Failed to save field '@field'!\n@tripal_message",
              array(
                '@field'          => $field_name,
                '@tripal_message' => $tripal_message,
              )
            )
          );
        }
        ob_get_clean();
      }
      break;
    case "object":
      // set through a function
      if ('Closure' == get_class($field_fetcher)) {
        $field_fetcher($data_type, $object, $field_name, 'create', $field_value);
      }
      break;
    default: 
      return brapi_raise_error_json(
        400,
        t(
          "Bad request! Invalid field '@field' configuration for data type '@type' (field type '@fetcher')!",
          array(
            '@field'   => $field_name,
            '@type'    => $data_type,
            '@fetcher' => $field_fetcher,
          )
        )
      );
  }
  
  return $value;
}

/**
 * Handles CRUD (Create-Read-Update-Delete) HTTP calls according to BrAPI
 * settings set in brapi_get_calls(). It also handles user persmission. Here is
 * how we map CRUD operations and HTTP methods:
 *
 * * Create: POST without an ID (a new ID is generated) or PUT with the new ID
 *   to use (if the ID is already in use, the object is updated);
 *
 * * Read: GET method only. Note that dedicated search calls exist to filter
 *   object using their fields and those calls support POST method;
 *
 * * Update: POST or PUT with an ID of an existing object;
 *
 * * Delete: DELETE with the ID of an existing object.
 *
 * @param string $call
 *   call name. Must be a key of the array returned by brapi_get_calls().
 *
 * @param array $actions
 *   an array with the following structure:
 *   @code
 *   array(
 *     'create' => function ($user_data, $id=NULL) {...},
 *     'read'   => function ($object) {...},
 *     'update' => function ($user_data, $object) {...},
 *     'delete' => function ($object) {...},
 *     'list'   => function () {...},
 *   );
 *   @endcode
 *   Keys are CRUD action to perform ('list' just returns a list of objects) and
 *   values are functions that return JSON data in a string.
 *
 * @param string $data_type
 *   name of the data type. See keys of the array returned by
 *   brapi_get_data_mapping().
 *
 * @param string $id
 *   an object identifier. Can be omitted or NULL (default). This ID may be used
 *   to load the corresponding object if needed by the operation/call.
 *
 * @see brapi_get_calls()
 * @see brapi_get_data_mapping()
 *
 * @ingroup brapi_api
 */
function brapi_process_crud($call, $actions, $data_type, $id = NULL) {

  $call_settings = brapi_get_calls()[$call];
  $data_settings = brapi_get_data_mapping()[$data_type];

  // check method
  if (!array_key_exists($_SERVER['REQUEST_METHOD'], $call_settings['methods'])) {
    return brapi_raise_error_json(403, t("Invalid HTTP method!"));
  }
  
  // check access permission
  if (!user_access($call_settings['methods'][$_SERVER['REQUEST_METHOD']])) {
    return brapi_raise_error_json(401, t("Access denied!"));
  }

  // setup default actions
  $default_actions = array(
    'create' => function ($user_data, $id=NULL) {
      return brapi_raise_error_json(501, t("Create not implemented!"));
    },
    'read'   => function ($object) {
      return brapi_raise_error_json(501, t("Get object not implemented!"));
    },
    'update' => function ($user_data, $object) {
      return brapi_raise_error_json(501, t("Update not implemented!"));
    },
    'delete' => function ($object) {
      return brapi_raise_error_json(501, t("Delete not implemented!"));
    },
    'list'   => function () {
      return brapi_raise_error_json(501, t("Listing not implemented!"));
    },
  );
  $actions = array_merge($default_actions, $actions);

  // get corresponding data object if one
  if (!empty($id)) {
    $selector = array(
      $data_settings['accession'] => $id,
    );
    $object = chado_generate_var($data_settings['table'], $selector);
    if (is_array($object)) {
      $object = current($object);
    }
  }

  // get user data if some
  $putdata_stream = fopen("php://input", "r");
  $user_data = '';
  while ($data = fread($putdata_stream, 1024)) {
    $user_data .= $data;
  }
  fclose($putdata_stream);
  if (!empty($user_data)) {
    $user_data = json_decode($user_data, TRUE);
    if (empty($user_data)) {
      $json_error_type = '';
      switch (json_last_error()) {
        case JSON_ERROR_NONE:
          $json_error_type = t('No JSON format error found but data might be empty');
          break;
        case JSON_ERROR_DEPTH:
          $json_error_type = t('Maximum stack depth exceeded');
          break;
        case JSON_ERROR_STATE_MISMATCH:
          $json_error_type = t('Underflow or the modes mismatch');
          break;
        case JSON_ERROR_CTRL_CHAR:
          $json_error_type = t('Unexpected control character found');
          break;
        case JSON_ERROR_SYNTAX:
          $json_error_type = t('Syntax error, malformed JSON');
          break;
        case JSON_ERROR_UTF8:
          $json_error_type = t('Malformed UTF-8 characters, possibly incorrectly encoded');
          break;
        default:
          $json_error_type = t('Unknown error');
          break;
      }
      return brapi_raise_error_json(
        400,
        t(
          "Invalid JSON data provided: @json_error",
          array('@json_error' => $json_error_type,)
        )
      );
    }
  }

  // check request type
  $error_message = '';
  $mode = '';
  try {
    switch ($_SERVER['REQUEST_METHOD']) {
      case 'GET':
        if (empty($id)) {
          // without ID
          $mode = 'list';
          return $actions['list']();
        }
        else {
          // with ID
          if ($object) {
            $mode = 'read';
            return $actions['read']($object);
          }
          else {
            // not found
            return brapi_raise_error_json(404, t("Object not found!"));
          }
        }
        break;
      case 'POST':
        if (empty($id)) {
          // without ID
          if (empty($user_data)) {
            return brapi_raise_error_json(400, t("Missing input data!"));
          }
          // create new object
          $mode = 'create';
          return $actions['create']($user_data);
        }
        else {
          // with ID
          if ($object) {
            if (empty($user_data)) {
              return brapi_raise_error_json(400, t("Missing input data!"));
            }
            // update object if exists
            $mode = 'update';
            return $actions['update']($user_data, $object);
          }
          else {
            // not found
            return brapi_raise_error_json(404, t("Object not found!"));
          }
        }
        break;
      case 'PUT':
        if (empty($id)) {
          // without ID: error
          return brapi_raise_error_json(400, t("Object identifier is missing!"));
        }
        else {
          // with ID
          if (empty($user_data)) {
            return brapi_raise_error_json(400, t("Missing input data!"));
          }
          if ($object) {
            // update object
            $mode = 'update';
            return $actions['update']($user_data, $object);
          }
          else {
            // create new object
            $mode = 'create';
            return $actions['create']($user_data, $id);
          }
        }
        break;
      case 'DELETE':
        if (empty($id)) {
          // without ID: error
          return brapi_raise_error_json(400, t("Object identifier is missing!"));
        }
        else {
          // with ID
          if ($object) {
            // delete object
            $mode = 'delete';
            return $actions['delete']($object);
          }
          else {
            // not found
            return brapi_raise_error_json(404, t("Object not found!"));
          }
        }
        break;
      default:
        return brapi_raise_error_json(500, t("Invalid HTTP method not checked properly!"));
    }
  }
  catch (Exception $e) {
    $error_message =
      t(
        "An exception occurred (mode @mode): @message",
        array('@mode' => $mode, '@message' => $e->getMessage())
      );
  }
  return brapi_raise_error_json(
    500,
    t(
      "CRUD error! @error",
      array('@error' => $error_message,)
    )
  );
}

/**
 * Prepare response metadata value.
 *
 * Compute page indexing. Take in account pagination arguments passed as
 * argument to Drupal (URL query string). If the requested page is outside range
 * the status will be set to reflect the issue.
 *
 * Supported GET parameters:
 * - page (integer)
 *   index of the requested page (first page having index '1')
 * - pageSize (integer)
 *   number of item on a single page
 *
 * @param integer $item_count
 *   Total number of item that the query could return (without pagination).
 *   Default: 1
 *
 * @return array
 *   a metadata hash containing the pagination and status data.
 *
 * @ingroup brapi_api
 */
function brapi_prepare_metadata($item_count = 1) {
  $metadata = array(
    'status' => array(),
  );

  if ($item_count) {
    $args = drupal_get_query_parameters();

    $page_size   = isset($args['pageSize']) ? $args['pageSize'] : FALSE;
    $page_number = isset($args['page']) ? $args['page'] : FALSE;

    if (!$page_size || !is_numeric($page_size) || ($page_size <= 0)) {
      if ($page_size) {
        $metadata['status'][] =
          array(
            'code' => 400,
            'message' => t(
              "Invalid page size specified ('@page_size')! Using default (@default_size).",
              array(
                '@page_size'    => htmlentities($page_size),
                '@default_size' => BRAPI_DEFAULT_PAGE_SIZE,
              )
            )
          );
      }
      $page_size = BRAPI_DEFAULT_PAGE_SIZE;
    }

    $page_count = ceil($item_count / $page_size);

    if ($page_number && is_numeric($page_number) && (0 < $page_number)) {
      $page_number = ceil($page_number); // convert to int
      if ($page_count < $page_number) {
        $metadata['status'][] =
          array(
            'code' => 404,
            'message' => t(
              "WARNING: page number (@page_number) out of bounds (1-@page_count).",
              array(
                '@page_number' => $page_number,
                '@page_count'  => $page_count,
              )
            )
          );
      }
    }
    else {
      if ($page_number) {
        $metadata['status'][] =
          array(
            'code' => 404,
            'message' => t(
              "Invalid page number specified ('@page_number')!",
              array(
                '@page_number' => htmlentities($page_number),
              )
            )
          );
      }
      $page_number = 1;
    }

    $metadata['pagination'] = array(
      'pageSize'    => $page_size,
      'currentPage' => $page_number,
      'totalCount'  => $item_count,
      'totalPages'  => $page_count,
    );
  }

  return $metadata;
}

/**
 * Outputs the JSON data corresponding to a BRAPI call.
 *
 * Helper function that formats the given call response into a standardized
 * hash with all requested elements.
 *
 * Supported GET parameters:
 * - debug (boolean)
 *   include debug information in output if some have been provided (see
 *   $debug_data parameter);
 * - pretty (boolean)
 *   if set to a true value, output a nice indented human-readable JSON
 *   structure.
 *
 * @param array $data
 *   Response data hash.
 *
 * @param $metadata
 *   A specific metadata hash if it has been used by the API call.
 *   If the response should not include any metadata, then set this parameter
 *   to an empty array.
 *   Default: array(
 *     'pagination' => (object) NULL,
 *     'status' => array()
 *   )
 *
 * @param array $debug_data
 *   an array/hash containing additional debugging information that should be
 *   output.
 *   Default: array()
 *
 * @return string
 *   a JSON string.
 *
 * @ingroup brapi_api
 */
function brapi_output_json(
    $data = array(),
    $metadata = array(
      'pagination' => NULL,
      'status'     => array()
    ),
    $debug_data = array()
  ) {

  $args = drupal_get_query_parameters();

  // check if have metadata (either default or specified non-empty one)
  if (count($metadata)) {
    // check if we got no pagination data and need to create one
    if (empty($metadata['pagination'])) {
      $metadata['pagination'] = json_decode('{}'); // we want an empty (JSON) hash, not an empty array
    }
    // combine data and metadata
    $output_data = array_merge(array('metadata' => $metadata), $data);
  }
  else {
    // only output data (no metadata)
    $output_data = $data;
  }

  if (isset($args['debug']) && isset($debug_data)) {
    $output_data['debug'] = implode("\n", $debug_data);
  }
  
  if (!isset($args['pretty']) || $args['pretty']) {
    drupal_add_http_header('Content-Type', 'application/json');
    echo json_encode($output_data, JSON_PRETTY_PRINT);
    return;
  }

  return drupal_json_output($output_data);
}

/**
 * Returns an error response.
 *
 * Helper function that formats the given error response into a standardized
 * hash with all requested elements.
 *
 * @param integer $http_error_code
 *   HTTP error code (usually 4xx or 5xx).
 *
 * @param string $message
 *   Error description in one or more sentences. It should have already been
 *   processed by t().
 *
 * @param array $data
 *   Response data hash if some.
 *   Default: array()
 *
 * @param array $debug_data
 *   an array/hash containing additional debugging information that should be
 *   output.
 *   Default: array()
 *
 * @return string
 *   a JSON string.
 *
 * @ingroup brapi_api
 */
function brapi_raise_error_json($http_error_code, $message, $data = array(), $debug_data = array()) {
  drupal_add_http_header('Status', $http_error_code);
  $metadata = array(
    'status' => array(
      array(
        'code' => $http_error_code,
        'message' => $message,
      )
    ),
  );
  return brapi_output_json($data, $metadata, $debug_data);
}

/**
 * Returns an error response.
 *
 * Helper function that returns the given data structure as an error without
 * processing the structure to add any field except the debug one.
 *
 * Supported GET parameters:
 * - debug boolean
 *   include debug information in output if some have been provided (see
 *   $debug_data parameter)
 *
 * @param integer $http_error_code
 *   HTTP error code (usually 4xx or 5xx).
 *
 * @param array $data
 *   Response data hash if some.
 *   Default: array()
 *
 * @param array $debug_data
 *   an array/hash containing additional debugging information that should be
 *   output.
 *   Default: array()
 *
 * @return string
 *   a JSON string.
 *
 * @ingroup brapi_api
 */
function brapi_raise_raw_error_json($http_error_code, $data = array(), $debug_data = NULL) {
  drupal_add_http_header('Status', $http_error_code);

  $args = drupal_get_query_parameters();
  if (isset($args['debug']) && isset($debug_data)) {
    $data = array_merge($data, array('debug' => $debug_data));
  }

  return drupal_json_output($data);
}
