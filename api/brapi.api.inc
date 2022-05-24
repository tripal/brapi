<?php

/**
 * @file
 * API functions provided by Plant Breeding API module.
 *
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

// Includes constants.
require_once 'brapi.const.inc';

/**
 * BrAPI exception class for errors.
 */
class BrapiException extends Exception {

  protected $httpErrorCode = 500;
  protected $data = array();
  protected $debugData = array();
  protected $isRaw = FALSE;

  /**
   * Constructs a BrAPI exception.
   */
  public function __construct(
    $http_error_code,
    $message,
    $data = array(),
    $debug_data = array(),
    $is_raw = FALSE
  ) {
    parent::__construct($message);
    $this->httpErrorCode = $http_error_code;
    $this->data = $data;
    $this->debugData = $debug_data;
    $this->isRaw = $is_raw;
  }

  /**
   * Returns exception HTTP status code.
   */
  public function getStatusCode() {
    return $this->httpErrorCode;
  }

  /**
   * Generates JSON output.
   */
  public function getJsonOutput() {
    if ($this->isRaw) {
      return brapi_raise_raw_error_json(
        $this->httpErrorCode,
        $this->data,
        $this->debugData
      );
    }
    else {
      return brapi_raise_error_json(
        $this->httpErrorCode,
        $this->getMessage(),
        $this->data,
        $this->debugData
      );
    }
  }

}

/**
 * Example implementation of hook_brapi_calls_alter.
 *
 * This example shows how to alter Breeding API call settings. It adds a new
 * version support (1.42) to the 'markers' call and sets the function that will
 * handle that version to function brapi_v1_markers_142_json($marker_id = NULL)
 * that must be defined elsewhere.
 *
 * @param array $brapi_calls
 *   An associative array where keys are BrAPI call names and values are
 *   call definitions.
 *
 * @see brapi_get_calls()
 *
 * @ingroup brapi_api
 */
function hook_brapi_calls_alter(&$brapi_calls) {
  $brapi_calls['markers']['api versions'][] = '1.42';
  $brapi_calls['markers']['callback versions']['1.42'] =
    'brapi_v1_markers_142_json';
}

/**
 * Example implementation of hook_brapi_data_mapping_alter.
 *
 * This example shows how to alter Breeding API data mapping. It adds 2 new
 * fields to the germplasm type ('internalName' and 'speciesNewFunc'), it
 * redefines the existing field 'seedSource' and replaces the 'species' field
 * by the new field 'speciesNewFunc'.
 *
 * @param array $brapi_data_mapping
 *   An associative array where keys are BrAPI entity types and values are
 *   data/field mapping definitions.
 *
 * @see brapi_get_data_mapping()
 *
 * @ingroup brapi_api
 */
function hook_brapi_data_mapping_alter(&$brapi_data_mapping) {
  $cv_settings = brapi_get_cv_settings();

  // Defines a new germplasm field called 'internalName' as the value of
  // stock.uniquename column.
  $brapi_data_mapping['germplasm']['internalName'] = 'uniquename';

  // Redefines the seedSource as a stock_cvterm which cvterm_id is part of the
  // allowed cvterm_id for seedSource as defined by the CV settings.
  $brapi_data_mapping['germplasm']['seedSource'] = array(
    'table' => 'stock_cvterm',
    'column' => 'cvterm_id',
    'selector' => array(
      'cvterm_id' => $cv_settings['SeedSource'],
      'is_not' => FALSE,
    ),
    'field_type' => 'cvterm',
    // We expect only one seed source.
    'return_array' => FALSE,
  );

  // Defines a new species field 'speciesNewFunc' as a function.
  $brapi_data_mapping['germplasm']['speciesNewFunc'] =
    function ($data_type, $stock, $field_name, $op, $values = NULL) use ($cv_settings) {
      $return = NULL;
      switch ($op) {
        case NULL:
          // This function supports 'read' and 'filter' operations.
          $return = array('read' => TRUE, 'filter' => TRUE);
          break;

        case 'read':
          // Just make sure our $stock is a germplasm.
          if (!empty($cv_settings['Germplasm'])
              && in_array($stock->type_id, (array) $cv_settings['Germplasm'])) {
            // Return its species name.
            $return = $stock->organism_id->species;
          }
          break;

        case 'filter':
          $return = array();
          // Get possible values as an array.
          // Values may be provided either as coma separated valuies in a string
          // or as an ready-to-use PHP array.
          if (is_array($values)) {
            $values = implode(',', $values);
          }
          $values = explode(',', $values);
          // For 'filter', $stock contains an array of stocks.
          foreach ($stock as $stock_to_check) {
            // Here goes some code to check if the stock species value is part
            // of the filter values (using 'OR' logic).
            if (!empty($stock_to_check->organism_id)
                && in_array($stock_to_check->organism_id->species, $values)) {
              $return[] = $stock_to_check;
            }
          }
          break;

        case 'create':
          // Not supported here but $values would contain the new field values.
        case 'update':
          // Not supported here but $values would contain the new field values.
        case 'delete':
        default:
          throw new BrapiException(501, t("Currently not supported!"));
      }
      return $return;
    };
  // Forces the use of this new function for the current species field.
  $brapi_data_mapping['germplasm']['species']['alias_for'] = 'speciesNewFunc';
}

/**
 * Example implementation of hook_brapi_object_selector_alter.
 *
 * This implementation adds a constraint of the uniquename of stock. Chado stock
 * with a uniquename not starting with '01' will be hidden.
 *
 * @param array $selector
 *   A selector following the same format as the one used by
 *   chado_select_record(). By default, it contains a key with a value that
 *   identifies the object to load (usually 'xxx_id' or 'uniquename').
 * @param array $context
 *   'call' key contains the BrAPI call name and 'data_type' contains the type
 *   of data that should be loaded (from brapi_get_data_mapping()).
 *
 * @see brapi_get_data_mapping()
 * @see chado_select_record()
 */
function hook_brapi_object_selector_alter(&$selector, $context) {
  if (('germplasm' == $context['data_type'])
      || ('sample' == $context['data_type'])) {
    // Check how the selector has been setup already.
    if (!empty($selector['uniquename'])) {
      // We already have a constraint on uniquename.
      if (is_array($selector['uniquename'])) {
        if (is_array($selector['uniquename'][0])) {
          // Complex filtering, add our filter.
          $selector['uniquename'][] =
            array(
              'op' => 'LIKE',
              'data' => '01%',
            );
        }
        else {
          // Multiple values, we can check each now.
          $valid_uniquenames = array();
          foreach ($selector['uniquename'] as $un) {
            if (0 === strpos($un, '01')) {
              $valid_uniquenames[] = $un;
            }
          }

          if (empty($valid_uniquenames)) {
            // No uniquename starting with 01, make sure no result will come.
            $selector = array('stock_id' => NULL);
          }
          else {
            $selector['uniquename'] = $valid_uniquenames;
          }
        }
      }
      else {
        // Single value, we can check now.
        if (0 !== strpos($selector['uniquename'], '01')) {
          // Not starting with 01, make sure no result will come.
          $selector = array('stock_id' => NULL);
        }
      }
    }
    else {
      // Not specified yet, add our filter.
      $selector['uniquename'] = array(
        array(
          'op' => 'LIKE',
          'data' => '01%',
        ),
      );
    }
  }
}

/**
 * Checks if an array contains the given values.
 *
 * @param mixed $mixed
 *   A value or an array of value to look for.
 * @param array $array
 *   The array to look into and check if the values are.
 * @param bool $must_have_all
 *   If true and $mixed is an array, all the values of $mixed must be found in
 *   the $array array.
 *
 * @return bool
 *   TRUE if the requested values have been found in the given array.
 */
function brapi_array_contains($mixed, $array, $must_have_all = TRUE) {
  $return_value = FALSE;
  if (is_array($mixed)) {
    foreach ($mixed as $value) {
      if ($must_have_all) {
        $return_value = $return_value && in_array($value, $array);
      }
      else {
        $return_value = $return_value || in_array($value, $array);
      }
    }
  }
  else {
    $return_value = in_array($mixed, $array);
  }

  return $return_value;
}

/**
 * Returns MCPD controlled vocabulary settings.
 *
 * Returns multi-crop passport descriptor controlled vocabulary settings used
 * by this Breeding API installation.
 *
 * @$reset bool
 *   Forces MCPD settings to be refreshed.
 *
 * @return array
 *   Keys are MCPD terms and values are arrays of associated chado cvterm_id.
 *
 * @see brapi_get_mcpd_mapping()
 * @see brapi_get_cv_settings()
 * @see brapi_get_data_mapping()
 *
 * @ingroup brapi_api
 */
function brapi_get_mcpd_settings($reset = FALSE) {
  static $mcpd_settings;
  static $mcpd_mapping;
  if (!$mcpd_settings || $reset) {
    if (!$mcpd_mapping) {
      $mcpd_mapping = brapi_get_mcpd_mapping();
    }

    $mcpd_settings = array();

    foreach ($mcpd_mapping as $brapi_term => $mapping) {
      $mcpd_settings[$brapi_term] = brapi_get_cvterm_id($mapping);
    }
  }

  return $mcpd_settings;
}

/**
 * Example implementation of hook_brapi_cv_settings_alter.
 *
 * This example shows how to alter Breeding API CV term settings. It changes the
 * CV term ID of a MCPD term named 'accessionNumber'.
 *
 * @param array $cv_settings
 *   An associative array where keys are BrAPI field names and values are
 *   Chado cvterm_ids or arrays of Chado cvterm_ids.
 *
 * @ingroup brapi_api
 */
function hook_brapi_cv_settings_alter(&$cv_settings) {
  $cv_settings['accessionNumber'] = brapi_get_cvterm_id(
    'accessionNumber',
    'brapi'
  );
}

/**
 * Reset CV settings to default.
 *
 * @return array
 *   Key are Breeding API term/field names and values are corresponding Chado
 *   cvterm_id.
 *
 * @see brapi_get_mcpd_settings()
 * @see brapi_get_data_mapping()
 *
 * @ingroup brapi_api
 */
function brapi_reset_cv_settings() {
  // Gets default.
  $brapi_cv_select = array('name' => BRAPI_CV);
  $brapi_cv = chado_select_record('cv', array('cv_id', 'name'), $brapi_cv_select);
  if (is_array($brapi_cv)) {
    $brapi_cv = current($brapi_cv);
  }

  if (!$brapi_cv) {
    drupal_set_message(t('BrAPI controlled vocabulary is missing! Re-install BrAPI module.'), 'error');
  }

  $brapi_cv_settings = array();
  foreach (array_keys(brapi_get_cv()) as $term_name) {
    // Get term from BrAPI CV.
    $cvterm_id = brapi_get_cvterm_id($term_name, $brapi_cv->name);
    if ($cvterm_id) {
      $brapi_cv_settings[$term_name] = $cvterm_id;
    }
    else {
      $brapi_cv_settings[$term_name] = 0;
      drupal_set_message(t('BrAPI term "@term_name" not found in BrAPI controlled vocabulary! You may need to re-install BrAPI module.', array('@term_name' => $term_name)), 'warning');
    }
  }

  // Merge with MCPD terms.
  $mcpd_cv_settings = brapi_get_mcpd_settings(TRUE);
  $settings = array_merge($brapi_cv_settings, $mcpd_cv_settings);

  variable_set(BRAPI_CV_SETTINGS, $settings);
  cache_set(BRAPI_CV_SETTINGS, $settings);
  return $settings;
}

/**
 * Returns CV settings.
 *
 * Returns an array containing the CV settings used by this Breeding API
 * installation.
 *
 * @param bool $reset
 *   Clear current settings and reload them from database.
 *
 * @return array
 *   Key are Breeding API term/field names and values are corresponding Chado
 *   cvterm_id.
 *
 * @see brapi_get_mcpd_settings()
 * @see brapi_get_data_mapping()
 *
 * @ingroup brapi_api
 */
function brapi_get_cv_settings($reset = FALSE) {
  static $settings;
  // If not initialized, get it from cache if available.
  if (!isset($settings) || $reset) {
    if (!$reset
        && ($cache = cache_get(BRAPI_CV_SETTINGS))
        && !empty($cache->data)) {
      $settings = $cache->data;
    }
    else {
      // Not available in cache, get it from saved settings.
      $settings = variable_get(BRAPI_CV_SETTINGS, array());
      if (!$settings) {
        // Not in saved settings, get defaults.
        $settings = brapi_reset_cv_settings();
      }
      drupal_alter('brapi_cv_settings', $settings);
      cache_set(BRAPI_CV_SETTINGS, $settings);
    }
  }
  else {
    drupal_alter('brapi_cv_settings', $settings);
  }

  return $settings;
}

/**
 * Returns the field fetcher for the given field.
 *
 * This function mainly handles field aliases.
 *
 * @param array $data_mapping
 *   The data mapping structure returned by brapi_get_data_mapping().
 * @param string $data_type
 *   Name of the object type.
 * @param string $field_name
 *   Name of the requested object field.
 *
 * @return mixed
 *   An object field fetcher settings used to get/set field values.
 *
 * @see brapi_get_data_mapping()
 */
function brapi_get_field_fetcher($data_mapping, $data_type, $field_name) {
  $field_fetcher = $data_mapping[$data_type]['fields'][$field_name];
  while (is_array($field_fetcher)
         && isset($field_fetcher['alias_for'])
         && $field_fetcher['alias_for']) {
    $field_fetcher =
      $data_mapping[$data_type]['fields'][$field_fetcher['alias_for']];
  }
  return $field_fetcher;
}

/**
 * Autocomplete for Chado CVs.
 *
 * Autocomplete helper function that returns suggestions of Chado CVs according
 * to user inputs.
 * It outputs a JSON array which keys are suggested CV names and cv_id as text
 * (computer-processable) and values are corresponding CV names
 * (human-readable).
 *
 * @param string $string
 *   Part of the CV name to use for the CV search.
 *
 * @ingroup brapi_api
 */
function brapi_cvs_autocomplete($string) {
  $matches = array();

  if ($string) {
    // Get last term entered after the last coma for completion.
    $items = array_map('trim', explode(',', $string));
    $last_item = array_pop($items);
    $prefix = implode(', ', $items);

    $sql_query = "
        SELECT
          cv.cv_id AS \"id\",
          cv.name AS \"cv\"
        FROM
          {cv} cv
        WHERE
          cv.name ILIKE :user_input
    ;";
    $results = chado_query($sql_query, array(':user_input' => $last_item . '%'));

    // Save the query to matches.
    foreach ($results as $row) {
      $value = (!empty($prefix) ? $prefix . ', ' : '') . $row->cv . ' (' . $row->id . ')';
      $text = $row->cv . ' (' . $row->id . ')';
      if ((!in_array($row->cv, $items)) && (!in_array($value, $items))) {
        $matches[$value] = check_plain($text);
      }
    }
  }

  // Return the result to the form in json.
  drupal_json_output($matches);
}

/**
 * Autocomplete for Chado CV terms.
 *
 * Autocomplete helper function that returns suggestions of Chado CV terms
 * according to user inputs.
 * It outputs a JSON array which keys are suggested CV term names with CV name
 * and cvterm_id as text (computer-processable) and values are corresponding CV
 * term names with CV (human-readable).
 *
 * @param string $string
 *   Part of the CV term name to use for the CV term search.
 *
 * @ingroup brapi_api
 */
function brapi_terms_autocomplete($string) {
  $matches = array();

  if ($string) {
    // Get last term entered after the last coma for completion.
    $items = array_filter(array_map(
      function ($a) {
        return trim(str_replace("\\,", ',', $a));
      },
      preg_split('/(?<!\\\\),/', $string)
    ));
    $last_item = array_pop($items);
    if (empty($last_item)) {
      drupal_json_output($matches);
      return;
    }

    $prefix = implode(', ', $items);

    $sql_query = "
        SELECT
          cvt.cvterm_id AS \"id\",
          cvt.name AS \"term\",
          cv.name AS \"cv\"
        FROM
          {cvterm} cvt
            INNER JOIN {cv} USING (cv_id)
        WHERE
          cvt.name ILIKE :user_input
    ;";
    $results = chado_query($sql_query, array(':user_input' => $last_item . '%'));

    // Save the query to matches.
    foreach ($results as $row) {
      $value = (!empty($prefix) ? $prefix . ', ' : '') . $row->term . ' (cv:' . $row->cv . '; id:' . $row->id . ')';
      $text = $row->term . ' (from ' . $row->cv . ' CV)';
      if ((!in_array($row->term, $items)) && (!in_array($value, $items))) {
        $matches[$value] = check_plain($text);
      }
    }
  }

  // Return the result to the form in json.
  drupal_json_output($matches);
}

/**
 * Autocomplete for filters.
 *
 * Autocomplete helper function that returns suggestions of filter values
 * to user inputs.
 * It outputs a JSON array which keys are suggested CV term names with CV name
 * and cvterm_id as text (computer-processable) and values are corresponding CV
 * term names with CV (human-readable).
 *
 * @param string $string
 *   Part of the CV term name to use for the CV term search.
 *
 * @ingroup brapi_api
 */
function brapi_filter_autocomplete($data_type, $field, $string) {
  $brapi_data_mapping = brapi_get_data_mapping();
  $matches = array();

  if ($string
      && isset($brapi_data_mapping[$data_type])
      && isset($brapi_data_mapping[$data_type]['fields'][$field])
      && ($field_fetcher = brapi_get_field_fetcher($data_mapping, $data_type, $field))
      && is_array($field_fetcher)
      && ('cvterm' == $field_fetcher['field_type'])) {

    // Get last term entered after the last coma for completion.
    $items = array_map('trim', explode(',', $string));
    $last_item = array_pop($items);
    $prefix = implode(', ', $items);

    ob_start();
    try {
      $records = chado_select_record(
         $field_fetcher['table'],
         array('DISTINCT ' . $field_fetcher['column']),
         $field_fetcher['selector']
      );
      $cvterm_ids = array();
      foreach ($records as $record) {
        $cvterm_ids[] = $record->{$field_fetcher['column']};
      }
      $cvterm_records = chado_select_record(
         'cvterm',
         array('name'),
         array('cvterm_id' => $cvterm_ids)
      );
      foreach ($cvterm_records as $record) {
        if (($last_item == '*')
            || (FALSE !== strpos($record->name, $last_item))) {
          $value = (!empty($prefix) ? $prefix . ', ' : '') . $record->name;
          $matches[$value] = check_plain($record->name);
        }
      }
    }
    catch (Exception $e) {
      drupal_set_message(t(
        "Failed to fetch field @field_name possible values! @error_message",
        array(
          '@field_name' => $field_name,
          '@error_message' => $e->getMessage() . "\n" . ob_get_clean(),
        )
      ));
    }
    ob_get_clean();
  }

  // Return the result to the form in json.
  drupal_json_output($matches);
}

/**
 * Helper function that returns the value of a given property for a given stock.
 *
 * @param int $stock_id
 *   The Chado stock_id value.
 * @param string $property_name
 *   Name of the CV term corresponding to the property to fetch.
 * @param string $cv_name
 *   Name of the CV the property term belongs to.
 *
 * @return string
 *   The property value.
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
  // Sort by rank.
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
 * Returns stock CV terms.
 *
 * Helper function that returns an array of CV terms associated to the given
 * stock.
 *
 * @param int $stock_id
 *    The Chado stock_id value.
 * @param array $term_selectors
 *   A hash containing any combination of the following keys:
 *   - 'cv' (string/array of string): one or more CHADO cv names;
 *   - 'cv_id' (int/array of int): one or more CHADO cv_id;
 *   - 'term_id' (int/array of int): one or more CHADO cvterm_id;
 *   - 'term' (string): a single CHADO cvterm name;
 *   - 'terms' (array of string): one or more CHADO cvterm names;
 *   - 'parent_term_id' (int/array of int): CHADO cvterm_id of one or
 *     more parent terms;
 *   - 'parent_term' (string): CHADO cvterm name of the parent cvterm;
 *   - 'parent_terms' (array of string): array of CHADO cvterm names of parents
 *     cvterms.
 *
 * @return array
 *   An array of Tripal CHADO CV term objects or an empty array if no matching
 *   term was found.
 *
 * @ingroup brapi_api
 */
function brapi_get_stock_cvterm_values($stock_id, $term_selectors) {

  $no_constraint = TRUE;
  // Check if we have a limited list of CV to look into.
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

  // Check if we got a list of term ids.
  $term_ids = array();
  if (!empty($term_selectors['term_id'])) {
    $no_constraint = FALSE;
    if (is_array($term_selectors['term_id'])) {
      $term_ids = array_merge($term_ids, $term_selectors['term_id']);
    }
    else {
      $term_ids[] = $term_selectors['term_id'];
    }
  }

  // Check if we got a list of term names.
  if (!empty($term_selectors['term']) || !empty($term_selectors['terms'])) {

    $no_constraint = FALSE;
    // Only work on one list of terms.
    if (empty($term_selectors['terms'])) {
      $term_selectors['terms'] = array();
    }
    if (!empty($term_selectors['term'])) {
      $term_selectors['terms'][] = $term_selectors['term'];
    }

    // Get term IDs.
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
        $term = tripal_get_cvterm(array('name' => $term_name));
        if ($term) {
          $term_ids[] = $term->cvterm_id;
        }
      }
    }

    // Make sure we restrict on cvterm_id even when no macth were found.
    if (!count($term_ids)) {
      $term_ids = array(0);
    }
  }

  // Check if we got a parent terms.
  $parent_term_ids = array();
  if (!empty($term_selectors['parent_term_id'])) {
    $no_constraint = FALSE;
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

  // Only work on one list.
  if (empty($term_selectors['parent_terms'])) {
    $term_selectors['parent_terms'] = array();
  }
  if (!empty($term_selectors['parent_term'])) {
    $term_selectors['parent_terms'][] = $term_selectors['parent_term'];
  }

  // Get parent term IDs from names.
  foreach ($term_selectors['parent_terms'] as $parent_term_name) {
    $no_constraint = FALSE;
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
      $term = tripal_get_cvterm(array('name' => $parent_term_name));
      if ($term) {
        $parent_term_ids[] = $term->cvterm_id;
      }
    }
  }

  // Now get descendant terms.
  if (count($parent_term_ids)) {
    $columns = array('subject_id');
    $values = array(
      'object_id' => $parent_term_ids,
      'type_id' => array(
        'cv_id' => array(
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

  // Get the list of selected terms associated to the given accession.
  $columns = array('cvterm_id');
  $values = array(
    'stock_id' => $stock_id,
  );

  // Check if just a CV ID has been specified.
  if ($no_constraint && count($cv_ids)) {
    // Get all terms from the given CV.
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
 * Returns a CV terms object.
 *
 * @param string $term_name
 *   Name of the CV term or a string containing its ID or its CV name.
 *   Supported format are (in check order):
 *   - <cvterm_id>
 *   - id:<cvterm_id>
 *   - <cvterm_name> (cv:<cv_name>)
 *   - <cvterm_name>.
 * @param string $cv
 *   Optional name of the CV (default: BRAPI_MULTICROP_PASSPORT_ONTOLOGY_CV).
 *
 * @return string
 *   Returns the corresponding CV term object or NULL if not found or multiple
 *   matches.
 *
 * @ingroup brapi_api
 */
function brapi_get_cvterm($term_name, $cv = BRAPI_MULTICROP_PASSPORT_ONTOLOGY_CV) {
  $selector = array();
  if (preg_match('/^\s*(\d+)\s*$/', $term_name, $matches)) {
    // Got cvterm_id.
    $selector['cvterm_id'] = $cleaned_term_name = $matches[1];
  }
  elseif (preg_match('/(?:^|\W)id:\s*(\d+)/', $term_name, $matches)) {
    // Got cvterm_id (method 2).
    $selector['cvterm_id'] = $cleaned_term_name = $matches[1];
  }
  elseif (preg_match('/(.+)\((?:.*\W|)cv:\s*([\w ]+)\)/', $term_name, $matches)) {
    // Find term from cv.
    $selector['name'] = $cleaned_term_name = trim($matches[1]);
    $selector['cv_id'] = array('name' => trim($matches[2]));
  }
  else {
    // We remove extra invalid (no id: or cv:) data in parenthesis if some.
    // Note: if we have a parenthesis matching at position 0, it means we
    // don't have anything before so we should not remove anything then.
    $cleaned_term_name = $term_name;
    if ($parenthesis_pos = strpos($cleaned_term_name, '(')) {
      $cleaned_term_name = substr($cleaned_term_name, 0, $parenthesis_pos);
    }
    $cleaned_term_name = trim($cleaned_term_name);
    $selector['name'] = $cleaned_term_name;
  }

  if ((FALSE !== $cv) && (NULL !== $cv)) {
    $selector['cv_id'] = array('name' => $cv);
  }

  // Try to get the CV term from Tripal.
  try {
    $cvterm = tripal_get_cvterm($selector);
  }
  catch (Exception $e) {
    $cvterm = NULL;
  }

  // If we did not find a term, check if we tried using a modified term and if
  // so, try original one.
  if (!$cvterm && ($cleaned_term_name != $term_name)) {
    try {
      // Restart with a new selector.
      $selector = array('name' => $term_name);
      // Check if a CV was specified.
      if ((FALSE !== $cv) && (NULL !== $cv)) {
        $selector['cv_id'] = array('name' => $cv);
      }
      $cvterm = tripal_get_cvterm($selector);
    }
    catch (Exception $e) {
      $cvterm = NULL;
    }
  }

  // Return the CV term.
  return $cvterm;
}

/**
 * Returns a CV terms database identifier.
 *
 * @param mixed $filter
 *   Can be either a string corresponding to a CV term name
 *   or a string containing its ID or its CV name,
 *     where supported format are (in check order):
 *     - <cvterm_id>
 *     - id:<cvterm_id>
 *     - <cvterm_name> (cv:<cv_name>)
 *     - <cvterm_name>
 *   or an array with one of the following keys:
 *     - 'name': string corresponding a term name or array of string
 *       corresponding to term names;
 *     - 'related': either the parent term name or an array with the fields
 *       'name' as a string for parent term name or an array of strings for a
 *       list of parent term names, 'cv' (string) for the CV the related terms
 *       belong to and 'relationship' (string) for the type of relationship.
 * @param string $cv
 *   Optional name of the CV (default: BRAPI_MULTICROP_PASSPORT_ONTOLOGY_CV).
 *   Set this parameter to FALSE (or NULL) if you do not want to specify the
 *   CV name (and discard default name).
 *
 * @return int
 *   Returns the corresponding cvterm_id or NULL if not found or multiple
 *   matches for a single name (when $filter is a string).
 *
 * @ingroup brapi_api
 */
function brapi_get_cvterm_id($filter, $cv = BRAPI_MULTICROP_PASSPORT_ONTOLOGY_CV) {

  $id_found = NULL;
  if (is_array($filter)) {
    if (isset($filter['cv']) && $filter['cv']) {
      $cv = $filter['cv'];
    }

    if (isset($filter['name'])) {
      $id_found = array();
      if (is_array($filter['name'])) {
        foreach ($filter['name'] as $term_name) {
          $cvterm = brapi_get_cvterm($term_name, $cv);
          if ($cvterm) {
            $id_found[] = $cvterm->cvterm_id;
          }
        }
      }
      else {
        $cvterm = brapi_get_cvterm($filter['name'], $cv);
        if ($cvterm) {
          $id_found = $cvterm->cvterm_id;
        }
      }
    }
    elseif (isset($filter['related'])) {
      if (is_array($filter['related'])) {
        if (isset($filter['related']['cv'])) {
          $cv = $filter['related']['cv'];
        }
        else {
          $cv = BRAPI_MULTICROP_PASSPORT_ONTOLOGY_CV;
        }

        if (isset($filter['related']['relationship'])) {
          $relationship = $filter['related']['relationship'];
        }
        else {
          $relationship = 'is_a';
        }

        if (is_array($filter['related']['name'])) {
          foreach ($filter['related']['name'] as $term_name) {
            $id_found[] = brapi_get_related_cvterm_id(
              $term_name, $cv, $relationship
            );
          }
        }
        else {
          $id_found = brapi_get_related_cvterm_id(
            $filter['related']['name'], $cv, $relationship
          );
        }
      }
      else {
        $id_found = brapi_get_related_cvterm_id($filter['related']);
      }
    }
    else {
      throw new Exception(t(
        "Invalid CV term filter: no valid key found in filter array!"
      ));
    }

    if (empty($id_found)) {
      $id_found = NULL;
    }
  }
  else {
    $cvterm = brapi_get_cvterm($filter, $cv);
    if ($cvterm) {
      $id_found = $cvterm->cvterm_id;
    }
  }

  return $id_found;
}

/**
 * Returns children CV terms of a parent CV term.
 *
 * Given a parent CV term name and an optional CV name, it returns a sorted
 * array of cvterm_id corresponding to the children CV terms.
 *
 * @param string $parent_cvterm
 *   name of the parent CV term.
 * @param string $cv
 *   name of the CV.
 *
 * @return array
 *   returns an array of sorted cvterm_id
 *
 * @ingroup brapi_api
 */
function brapi_get_related_cvterm_id($parent_cvterm, $cv = BRAPI_MULTICROP_PASSPORT_ONTOLOGY_CV, $relationship = 'is_a') {
  // Get parent term IDs from names.
  $cvterm_id = brapi_get_cvterm_id($parent_cvterm, $cv);

  // Now get descendant terms.
  $columns = array('subject_id');
  $values = array(
    'object_id' => $cvterm_id,
    'type_id' => array(
      'cv_id' => array(
        'name' => 'relationship',
      ),
      'name' => $relationship,
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

  if (empty($term_ids)) {
    // We add a fake cvterm_id if none found because returning an empty array
    // in future (Tripal) queries would not be taken into account.
    // Returning a 0 will force Tripal to use the cvterm_id parameter in its
    // queries.
    $term_ids[] = 0;
  }

  sort($term_ids);

  return $term_ids;
}

/**
 * Parses a date from a database value.
 *
 * Parses a date from a database value using the selected date format according
 * to the specified format or, if not set, BrAPI settings.
 *
 * @param string $stored_value
 *   Database value to parse.
 * @param string $format
 *   Format to use for parsing. Supported formats are xsd, timestamp and custom.
 *
 * @return string
 *   The date in BrAPI format (xsd:date, YYYY-MM-DD).
 *
 * @ingroup brapi_api
 */
function brapi_parse_date($stored_value, $format = NULL) {
  static $date_setting;

  if (!$date_setting) {
    $date_setting = variable_get(BRAPI_DATE_FORMAT, '');
  }

  if (!$format) {
    $format = $date_setting;
  }

  switch ($format) {
    case 'timestamp':
      $parsed_date = date(BRAPI_XSD_DATE_FORMAT, $stored_value);
      break;

    case 'custom':
      $date_format = variable_get(BRAPI_CUSTOM_DATE_FORMAT, '');
      if (!$date_format) {
        $date_format = BRAPI_XSD_DATE_FORMAT;
      }
      $date = date_create_from_format($date_format, $stored_value);
      $parsed_date = $date->format(BRAPI_XSD_DATE_FORMAT);
      break;

    case 'xsd':

    default:
      $parsed_date = $stored_value;
      break;
  }

  return $parsed_date;
}

/**
 * Formats the given BrAPI date.
 *
 * Formats the given BrAPI date (xsd format) into the requested format or the
 * default date format setup in the settings.
 *
 * @param string $brapi_date
 *   BrAPI date (xsd format).
 * @param string $format
 *   Requested format. Supported formats are xsd, timestamp and custom.
 *
 * @return string
 *   Date formatted in the requested format.
 *
 * @ingroup brapi_api
 */
function brapi_encode_date($brapi_date, $format = NULL) {
  static $date_setting;

  if (!$date_setting) {
    $date_setting = variable_get(BRAPI_DATE_FORMAT, '');
  }

  if (!$format) {
    $format = $date_setting;
  }

  $date = date_create_from_format(BRAPI_XSD_DATE_FORMAT, $brapi_date);

  switch ($format) {
    case 'timestamp':
      $encoded_date = $date->format('U');
      // Clear time part.
      $encoded_date -= $encoded_date % 86400;
      break;

    case 'custom':
      $date_format = variable_get(BRAPI_CUSTOM_DATE_FORMAT, '');
      if (!$date_format) {
        $date_format = BRAPI_XSD_DATE_FORMAT;
      }
      $encoded_date = $date->format($date_format);
      break;

    case 'xsd':
    default:
      $encoded_date = $brapi_date;
      break;
  }

  return $encoded_date;
}

/**
 * Returns a Tripal field selector.
 *
 * Returns a field selector that can be used by Tripal functions like
 * @link http://api.tripal.info/api/tripal/tripal_core!api!tripal_core.chado_query.api.inc/function/chado_select_record/2.x* chado_select_record() @endlink
 * .
 *
 * @param string $data_type
 *   Name of the data type. See keys of the array returned by
 *   brapi_get_data_mapping().
 * @param string $field_name
 *   A field name such as the one returned as keys of the field 'fields' of a
 *   datatype returned by brapi_get_data_mapping().
 * @param mixed $values
 *   Value or array of values that must match the field.
 *
 * @return mixed
 *   Value or array of values to use to identify the field record.
 */
function brapi_get_field_selector($data_type, $field_name, $values) {
  static $data_mapping;
  if (!isset($data_mapping)) {
    $data_mapping = brapi_get_data_mapping();
  }

  $field_fetcher = brapi_get_field_fetcher(
    $data_mapping,
    $data_type,
    $field_name
  );

  if (isset($field_fetcher['foreign_key'])) {
    $foreign_key = $field_fetcher['foreign_key'];
  }
  else {
    $foreign_key = $data_mapping[$data_type]['identifier'];
  }
  return array_merge(
    array($foreign_key => $values),
    $field_fetcher['selector']
  );
}

/**
 * Decode a database value.
 *
 * Decode a database value according to its type and turns it into a BrAPI
 * value.
 *
 * @param string $value
 *   The given raw database value.
 * @param string $type
 *   Value type.
 *
 * @return string
 *   Returns the decoded value according to the given type.
 *
 * @Throws Exception
 *   Throw an exception is the value type is not correct.
 */
function brapi_decode_value($value, $type) {
  switch ($type) {
    case 'bool':
      $value = (bool) $value;
      break;

    case 'int':
      $value = (int) $value;
      break;

    case 'float':
      $value = (float) $value;
      break;

    case 'date':
      $value = brapi_parse_date($value);
      break;

    case 'string':
      break;

    case 'cvterm':
      if ($term = tripal_get_cvterm(array('cvterm_id' => $value))) {
        $value = $term->name;
      }
      else {
        $value = NULL;
      }
      break;

    default:
      throw new Exception(
        t(
          "Failed to decode value ('@value')! Invalid value type @type!",
          array(
            '@type' => $type,
            '@value' => $value,
          )
        )
      );
  }
  return $value;
}

/**
 * Encode a BrAPI value.
 *
 * Encode a BrAPI value according to its type and turns it into a database
 * value.
 *
 * @param string $value
 *   The given BrAPI value.
 * @param string $type
 *   Value type.
 *
 * @return string
 *   Returns the encoded value according to the given type.
 *
 * @Throws Exception
 *   Throw an exception is the value type is not correct.
 */
function brapi_encode_value($value, $type) {
  switch ($type) {
    case 'bool':
      $value = $value ? '1' : '0';
      break;

    case 'date':
      $value = brapi_encode_date($value);
      break;

    case 'cvterm':
      if ($term = tripal_get_cvterm(array('name' => $value))) {
        $value = $term->cvterm_id;
      }
      else {
        $value = NULL;
      }
      break;

    case 'int':
    case 'float':
    case 'string':
      // For other types, values are left as they are.
      break;

    default:
      throw new Exception(
        t(
          "Failed to encode value ('@value')! Invalid value type @type!",
          array(
            '@type' => $type,
            '@value' => $value,
          )
        )
      );
  }
  return $value;
}

/**
 * Returns a given field value.
 *
 * Returns a breeding API field value according to a given data type, a given
 * loaded object and a given field name. Supported fields are defined in the
 * Breeding API module data mapping.
 *
 * @param string $data_type
 *   Name of the data type. See keys of the array returned by
 *   brapi_get_data_mapping().
 * @param object $object
 *   A Tripal object like a Tripal stock for instance.
 * @param string $field_name
 *   A field name such as the one returned as keys of the field 'fields' of a
 *   datatype returned by brapi_get_data_mapping().
 * @param mixed $skip_fallback
 *   If set to TRUE (bool), no fallback field is processed, if set to FALSE
 *   (bool), all available fallback fields are processed. If set to and array of
 *   string representing fallback field names, evry field name specified will
 *   _not_ be processed. The array type of argument is also used internaly to
 *   avoid infinite loop fallbacks.
 *
 * @return mixed
 *   returns an array if the mapping allows several values or the last value
 *   ortherwise.
 *
 * @Throws Exception
 *   Thrown if an error occured while querying the database.
 *
 * @see brapi_get_data_mapping()
 *
 * @ingroup brapi_api
 */
function brapi_get_field(
  $data_type,
  $object,
  $field_name,
  $skip_fallback = FALSE) {

  static $data_mapping;
  if (!isset($data_mapping)) {
    $data_mapping = brapi_get_data_mapping();
  }
  // Make sure we got a mapping defined.
  if (!isset($data_mapping[$data_type])) {
    throw new BrapiException(
      400,
      t(
        "Bad request! Unknown data type '@type'!",
        array('@type' => $data_type)
      )
    );
  }

  if (!isset($data_mapping[$data_type]['fields'][$field_name])) {
    throw new BrapiException(
      400,
      t(
        "Bad request! Unknown field '@field' for data type '@type'!",
        array('@field' => $field_name, '@type' => $data_type)
      )
    );
  }

  // Get values using mapping.
  $field_fetcher = brapi_get_field_fetcher($data_mapping, $data_type, $field_name);
  $value = '';
  switch (gettype($field_fetcher)) {
    case 'string':
      // Get from object.
      $value = $object->$field_fetcher;
      break;

    case 'array':
      // Get from database using mapping config.
      if (isset($field_fetcher['object_key'])) {
        $identifier = $object->{$field_fetcher['object_key']};
        if (is_object($identifier)) {
          $identifier = $identifier->{$field_fetcher['foreign_key']};
        }
      }
      else {
        $identifier = $object->{$data_mapping[$data_type]['identifier']};
      }

      $selector = brapi_get_field_selector($data_type, $field_name, $identifier);

      // We capture output buffer for Tripal error messages.
      ob_start();
      try {
        $records = chado_select_record(
           $field_fetcher['table'],
           array($field_fetcher['column']),
           $selector
        );
      }
      catch (Exception $e) {
        throw new Exception(t(
          "Failed to fetch field @field_name value for object type @object_type! @error_message",
          array(
            '@field_name' => $field_name,
            '@object_type' => $data_type,
            '@error_message' => $e->getMessage() . "\n" . ob_get_clean(),
          )
        ));
      }
      ob_get_clean();

      $values = array();
      if (FALSE !== $records) {
        // Force Tripal return value to be an array for easier management.
        if (!is_array($records)) {
          $records = array($records);
        }
        foreach ($records as $record) {
          $current_value = $record->{$field_fetcher['column']};
          // Check type and return the appropriate conversion.
          try {
            $current_value = brapi_decode_value($current_value, $field_fetcher['field_type']);
          }
          catch (Exception $e) {
            throw new BrapiException(
              400,
              t(
                "Bad request! Invalid field '@field' configuration for data type '@type' (returns field data type '@field_type')!\n@error_message",
                array(
                  '@field'         => $field_name,
                  '@type'          => $data_type,
                  '@field_type'    => $field_fetcher['field_type'],
                  '@error_message' => $e->getMessage(),
                )
              )
            );
          }
          $values[] = $current_value;
        }
      }

      // Check for fallback.
      if (empty($values)
          && isset($field_fetcher['fallback'])
          && (TRUE !== $skip_fallback)) {
        // Fallback.
        $fallback_stack = $field_fetcher['fallback'];
        if (!is_array($fallback_stack)) {
          $fallback_stack = array($field_fetcher['fallback']);
        }
        // Get what should be skipped.
        $skip_fallback_array = $skip_fallback;
        if (FALSE === $skip_fallback_array) {
          $skip_fallback_array = array();
        }
        // Just make sure we won't process current field again.
        array_push($skip_fallback_array, $field_name);

        // Process fallback fields until we got something or if we processed
        // all the fallbacks.
        while (empty($values) && !empty($fallback_stack)) {
          $fallback = array_shift($fallback_stack);
          // Skip this fallback?
          if ($fallback && !in_array($fallback, $skip_fallback_array)) {
            array_push($skip_fallback_array, $fallback);
            $values = brapi_get_field($data_type, $object, $fallback, $skip_fallback_array);
            $values = (is_array($values) ? $values : array($values));
          }
        }
      }

      // Check if array requested.
      if (isset($field_fetcher['return_array'])
          && $field_fetcher['return_array']) {
        // Multiple values.
        if (empty($values)) {
          $value = NULL;
        }
        else {
          $value = $values;
        }
      }
      else {
        // A single value.
        if (empty($values)) {
          $value = NULL;
        }
        else {
          $value = current($values);
        }
      }
      break;

    case 'object':
      // Get from a function.
      if (is_callable($field_fetcher)) {
        $value = $field_fetcher($data_type, $object, $field_name, 'read');
      }
      break;

    default:
      throw new BrapiException(
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
 * Sets a given field value.
 *
 * Set the value(s) of a field of a breeding data object. Multiple values can be
 * assigned to a same field (for instance several CV terms to attach to an
 * object). When updating an array of values, if $update is set to TRUE,
 * previous values are removed and only the new ones are set.
 *
 * @param string $data_type
 *   Name of the data type. See keys of the array returned by
 *   brapi_get_data_mapping().
 * @param object $object
 *   A Tripal object like a Tripal stock for instance.
 * @param string $field_name
 *   A field name such as the one returned as keys of the field 'fields' of a
 *   datatype returned by brapi_get_data_mapping().
 * @param mixed $field_value
 *   New value or array of values for the given field.
 * @param bool $update
 *   If FALSE, only tries to insert a new record. If TRUE, updates existing
 *   record by removing previous value(s) and if missing, creates a new one.
 *
 * @see brapi_get_data_mapping()
 *
 * @ingroup brapi_api
 */
function brapi_set_field(
  $data_type,
  &$object,
  $field_name,
  &$field_value,
  $update = TRUE) {

  $data_mapping = brapi_get_data_mapping();
  // Make sure we got a mapping defined.
  if (!isset($data_mapping[$data_type])) {
    throw new BrapiException(
      400,
      t(
        "Bad request! Unknown data type '@type'!",
        array('@type' => $data_type)
      )
    );
  }
  if (!isset($data_mapping[$data_type]['fields'][$field_name])) {
    throw new BrapiException(
      400,
      t(
        "Bad request! Unknown field '@field' for data type '@type'!",
        array('@field' => $field_name, '@type' => $data_type)
      )
    );
  }

  // Set values using mapping.
  $field_fetcher = brapi_get_field_fetcher($data_mapping, $data_type, $field_name);
  switch (gettype($field_fetcher)) {
    case "string":
      // Set through object.
      $object->$field_fetcher = $field_value;
      // @todo: $object->save(); ?
      break;

    case "array":
      // Set using mapping config.
      if (is_array($field_value)) {
        // Multiple values...
        // Check for ranking.
        $rank = 0;
        if ($field_fetcher['use_ranking']) {
          // We use ranking.
          $ranking = array('rank' => &$rank);
        }
        else {
          $ranking = array();
        }
        if ($update) {
          // Remove previous values.
          // We capture output buffer for Tripal error messages.
          ob_start();
          $selector = brapi_get_field_selector($data_type, $field_name, $object->{$data_mapping[$data_type]['identifier']});
          $deleted_record = chado_delete_record(
            $field_fetcher['table'],
            $selector
          );
          if (!$deleted_record) {
            $tripal_message = ob_get_clean();
            throw new BrapiException(
              500,
              t(
                "Unable to remove previous field values before update!\n@tripal_message",
                array('@tripal_message' => $tripal_message)
              )
            );
          }
          ob_get_clean();
          // Previous values removed.
        }
        // Insert new values.
        foreach ($field_value as $single_value) {
          // @todo Check value type. Do some conversion if needed and possible.
          // We capture output buffer for Tripal error messages.
          ob_start();
          $selector = brapi_get_field_selector($data_type, $field_name, $object->{$data_mapping[$data_type]['identifier']});
          try {
            $single_value = brapi_encode_value($single_value, $data_mapping[$data_type]['fields'][$field_name]['field_type']);
          }
          catch (Exception $e) {
            throw new BrapiException(
              400,
              t(
                "Bad request! Invalid field '@field' configuration for data type '@type' (returns field data type '@field_type')!\n@error_message",
                array(
                  '@field'         => $field_name,
                  '@type'          => $data_type,
                  '@field_type'    => $field_fetcher['field_type'],
                  '@error_message' => $e->getMessage(),
                )
              )
            );
          }
          if (!chado_insert_record(
             $field_fetcher['table'],
             array_merge(
               $selector,
               array(
                 $field_fetcher['column'] => $single_value,
               ),
               $ranking
             )
          )) {
            $tripal_message = ob_get_clean();
            throw new BrapiException(
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
        // Single value...
        if (isset($field_fetcher['foreign_key'])) {
          $foreign_key = $field_fetcher['foreign_key'];
        }
        else {
          $foreign_key = $data_mapping[$data_type]['identifier'];
        }
        if (isset($field_fetcher['object_key'])) {
          $identifier = $object->{$field_fetcher['object_key']};
        }
        else {
          $identifier = $object->{$data_mapping[$data_type]['identifier']};
        }
        // Check value type and do some conversion if needed.
        try {
          $field_value = brapi_encode_value($field_value, $data_mapping[$data_type]['fields'][$field_name]['field_type']);
        }
        catch (Exception $e) {
          throw new BrapiException(
            400,
            t(
              "Bad request! Invalid field '@field' configuration for data type '@type' (returns field data type '@field_type')!\n@error_message",
              array(
                '@field'         => $field_name,
                '@type'          => $data_type,
                '@field_type'    => $field_fetcher['field_type'],
                '@error_message' => $e->getMessage(),
              )
            )
          );
        }
        // We capture output buffer for Tripal error messages.
        ob_start();
        $updated_record = FALSE;
        if ($update) {
          $updated_record = chado_update_record(
             $field_fetcher['table'],
             array(
               $foreign_key => $identifier,
             ),
             array_merge(
               array(
                 $field_fetcher['column'] => $field_value,
               ),
               $field_fetcher['selector']
             )
          );
        }
        if (!$updated_record && !chado_insert_record(
           $field_fetcher['table'],
           array_merge(
             array(
               $foreign_key => $identifier,
             ),
             array(
               $field_fetcher['column'] => $field_value,
             ),
             $field_fetcher['selector']
           )
        )) {
          $tripal_message = ob_get_clean();
          throw new BrapiException(
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
      // Set through a function.
      if ('Closure' == get_class($field_fetcher)) {
        $field_fetcher($data_type, $object, $field_name, 'create', $field_value);
      }
      break;

    default:
      throw new BrapiException(
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
 * Handles CRUD calls.
 *
 * Handles CRUD (Create-Read-Update-Delete) HTTP calls according to BrAPI
 * settings set in brapi_get_calls(). It also handles user permissions. Here is
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
 *   Call name. Must be a key of the array returned by brapi_get_calls().
 * @param array $actions
 *   An array with the following structure:
 *     array(
 *       'create' => function ($user_data, $id=NULL) {...},
 *       'read'   => function ($object) {...},
 *       'update' => function ($user_data, $object) {...},
 *       'delete' => function ($object) {...},
 *       'list'   => function () {...},
 *     );
 *   Keys are CRUD action to perform ('list' just returns a list of objects) and
 *   values are functions that return JSON data in a string.
 * @param string $data_type
 *   Name of the data type. See keys of the array returned by
 *   brapi_get_data_mapping().
 * @param string $id
 *   An object identifier. Can be omitted or NULL (default). This ID may be used
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
  $storage_options = variable_get(BRAPI_STORAGE_OPTIONS, array());

  // Check method.
  if (!isset($call_settings['methods'][$_SERVER['REQUEST_METHOD']])) {
    throw new BrapiException(403, t("Invalid HTTP method!"));
  }

  // Check access permission.
  if (!user_access($call_settings['methods'][$_SERVER['REQUEST_METHOD']])) {
    throw new BrapiException(401, t("Access denied!"));
  }

  // Setup default actions.
  $default_actions = array(
    'create' => function ($user_data, $id = NULL) {
      throw new BrapiException(501, t("Create not implemented!"));
    },
    'read'   => function ($object) {
      throw new BrapiException(501, t("Get object not implemented!"));
    },
    'update' => function ($user_data, $object) {
      throw new BrapiException(501, t("Update not implemented!"));
    },
    'delete' => function ($object) {
      throw new BrapiException(501, t("Delete not implemented!"));
    },
    'list'   => function () {
      throw new BrapiException(501, t("Listing not implemented!"));
    },
  );
  $actions = array_merge($default_actions, $actions);

  // Get corresponding data object if one.
  if (!empty($id)) {
    $use_id = isset($storage_options['dbid'])
      && ('chado_id' == $storage_options['dbid']);
    $id_field = $use_id ?
      $data_settings['identifier']
      : $data_settings['accession'];
    $selector = array(
      $id_field => $id,
    );
    // Apply pre-filtering.
    if (!empty($data_settings['selector'])) {
      $selector = array_merge($selector, $data_settings['selector']);
    }
    // Allow other module interactions.
    $context = array('call' => $call, 'data_type' => $data_type);
    drupal_alter('brapi_object_selector', $selector, $context);
    $object = chado_generate_var($data_settings['table'], $selector);
    if (is_array($object)) {
      $object = current($object);
    }
  }

  // Get user data if some.
  try {
    $user_data = brapi_get_user_json_input();
  }
  catch (Exception $e) {
    throw new BrapiException(
      400,
      $e->getMessage()
    );
  }

  // Check request type.
  $error_message = '';
  $mode = '';
  try {
    switch ($_SERVER['REQUEST_METHOD']) {
      case 'GET':
        if (empty($id)) {
          // Without ID.
          $mode = 'list';
          return $actions['list']();
        }
        else {
          // With ID.
          if ($object) {
            $mode = 'read';
            return $actions['read']($object);
          }
          else {
            // Not found.
            throw new BrapiException(404, t("Object not found!"));
          }
        }
        break;

      case 'POST':
        if (empty($id)) {
          // Without ID.
          if (empty($user_data)) {
            throw new BrapiException(400, t("Missing input data!"));
          }
          // Create new object.
          $mode = 'create';
          return $actions['create']($user_data);
        }
        else {
          // With ID.
          if ($object) {
            if (empty($user_data)) {
              throw new BrapiException(400, t("Missing input data!"));
            }
            // Update object if exists.
            $mode = 'update';
            return $actions['update']($user_data, $object);
          }
          else {
            // Not found.
            throw new BrapiException(404, t("Object not found!"));
          }
        }
        break;

      case 'PUT':
        if (empty($id)) {
          // Without ID: error.
          throw new BrapiException(400, t("Object identifier is missing!"));
        }
        else {
          // With ID.
          if (empty($user_data)) {
            throw new BrapiException(400, t("Missing input data!"));
          }
          if ($object) {
            // Update object.
            $mode = 'update';
            return $actions['update']($user_data, $object);
          }
          else {
            // Create new object.
            $mode = 'create';
            return $actions['create']($user_data, $id);
          }
        }
        break;

      case 'DELETE':
        if (empty($id)) {
          // Without ID: error.
          throw new BrapiException(400, t("Object identifier is missing!"));
        }
        else {
          // With ID.
          if ($object) {
            // Delete object.
            $mode = 'delete';
            return $actions['delete']($object);
          }
          else {
            // Not found.
            throw new BrapiException(404, t("Object not found!"));
          }
        }
        break;

      default:
        throw new BrapiException(500, t("Invalid HTTP method not checked properly!"));
    }
  }
  catch (BrapiException $be) {
    throw $be;
  }
  catch (Exception $e) {
    $error_message =
      t(
        "An exception occurred (mode @mode): @message",
        array('@mode' => $mode, '@message' => $e->getMessage())
      );
  }

  throw new BrapiException(
    500,
    t(
      "CRUD error! @error",
      array('@error' => $error_message)
    )
  );
}

/**
 * Extracts user arguments for current call.
 *
 * Returns a hash containing URL arguments excluding global arguments such as
 * pagging data, output formatting and such.
 *
 * @return array
 *   Keys are argument names and values are corresponding argument values.
 *
 * @ingroup brapi_api
 */
function brapi_get_url_call_args() {
  $args = drupal_get_query_parameters();
  unset(
    $args['page'],
    $args['pageSize'],
    $args['pretty'],
    $args['debug']
  );
  return $args;
}

/**
 * Add fields specified by the 'include' argument to a field list.
 *
 * Returns an array with all the given fields plus the ones specified by the
 * include argument in the URL.
 *
 * @return array
 *   a list of fields.
 *
 * @ingroup brapi_api
 */
function brapi_merge_fields_to_include($field_list) {
  $args = brapi_get_url_call_args();
  if (isset($args['include'])) {
    // Make sure we start working on a string even with multiple values.
    if (is_array($args['include'])) {
      $args['include'] = implode(',', $args['include']);
    }
    // Turn includes into an array.
    $args['include'] = array_map('trim', explode(',', $args['include']));
    $field_list = array_filter(array_unique(array_merge($field_list, $args['include'])));
  }
  return $field_list;
}

/**
 * Returns a PHP structure containing user input (POST, PUT) for a call.
 *
 * @return array
 *   JSON data transcrypted into a PHP structure.
 *
 * @Throws Exception
 *   if JSON parsing fails. Provides JSON parsing last error as message.
 *
 * @ingroup brapi_api
 */
function brapi_get_user_json_input() {
  static $user_data;
  if (!isset($user_data)) {
    $user_data = array();
    // Read stream from user JSON POST/PUT data.
    $user_data_string = file_get_contents('php://input');
    // Tries to decode JSON data if some.
    if (!empty($user_data_string)) {
      $user_data = json_decode($user_data_string, TRUE);
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
        $user_data = array();
        // Throws JSON parsing error.
        throw new Exception(t(
          "Invalid JSON data provided: @json_error",
          array('@json_error' => $json_error_type)
        ));
      }
    }

    watchdog(
      'brapi',
      "Raw data received (POST/PUT):\n|@raw|\n\nJSON data parsed:\n@json",
      array(
        '@raw' => (isset($user_data_string) ? $user_data_string : ''),
        '@json' => print_r($user_data, TRUE),
      ),
      WATCHDOG_DEBUG,
      NULL
    );
  }
  return $user_data;
}

/**
 * Prepare response metadata value.
 *
 * Initializes default metadata fields and compute page indexing. Take in
 * account pagination arguments passed as argument to Drupal (URL query string).
 * If the requested page is outside range the status will be set to reflect the
 * issue.
 *
 * Supported GET (URL) parameters:
 * - page (int)
 *   index of the requested page (first page having index '1')
 * - pageSize (int)
 *   number of item on a single page
 *
 * @param int $item_count
 *   Total number of item that the query could return (without pagination).
 *   If set to 0, no pagination data is issued (ie. 'pagination' => NULL).
 *
 * @return array
 *   a metadata hash containing the pagination, status and datafiles data.
 *
 * @ingroup brapi_api
 */
function brapi_prepare_metadata($item_count = 1) {
  $metadata = array(
    'status' => array(),
    'pagination' => array(
      'pageSize'    => 0,
      'currentPage' => 0,
      'totalCount'  => 0,
      'totalPages'  => 0,
    ),
    'datafiles' => array(),
  );

  if ($item_count) {
    $args = drupal_get_query_parameters();

    $page_size   = isset($args['pageSize']) ? $args['pageSize'] : FALSE;
    $page_number = isset($args['page']) ? $args['page'] : FALSE;

    // Check from POST data.
    try {
      $user_data = brapi_get_user_json_input();
      if (isset($user_data['pageSize'])) {
        $page_size = $user_data['pageSize'];
      }
      if (isset($user_data['page'])) {
        $page_number = $user_data['page'];
      }
    }
    catch (Exception $e) {
      // Ignore error and just skip user data. It will be handled elsewhere.
    }

    // Set default page size if needed.
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
            ),
          );
      }
      $page_size = BRAPI_DEFAULT_PAGE_SIZE;
    }

    $page_count = ceil($item_count / $page_size);

    if ((FALSE !== $page_number)
        && preg_match('/^\d+$/', $page_number)) {
      // Convert in int (removes leading "0" if some).
      $page_number = floor($page_number);
      if ($page_count <= $page_number) {
        $metadata['status'][] = array(
          'code' => 404,
          'message' => t(
            "WARNING: page number (@page_number) out of bounds (0-@last_page).",
            array(
              '@page_number' => $page_number,
              '@last_page'   => $page_count - 1,
            )
          ),
        );
      }
    }
    elseif (FALSE !== $page_number) {
      $metadata['status'][] =
        array(
          'code' => 404,
          'message' => t(
            "Invalid page number specified ('@page_number')!",
            array(
              '@page_number' => htmlentities($page_number),
            )
          ),
        );
      $page_number = 0;
    }
    else {
      $page_number = 0;
    }

    $metadata['pagination'] = array(
      'pageSize'    => intval($page_size),
      'currentPage' => intval($page_number),
      'totalCount'  => intval($item_count),
      'totalPages'  => intval($page_count),
    );
  }

  return $metadata;
}

/**
 * Prepares the JSON data corresponding to a BRAPI call.
 *
 * Helper function that formats the given call response into a standardized
 * hash with all requested elements.
 *
 * Supported GET parameters:
 * - debug (bool)
 *   include debug information in output if some have been provided (see
 *   $debug_data parameter);
 *
 * @param array $data
 *   Response data hash.
 * @param array $metadata
 *   A specific metadata hash if it has been used by the API call.
 *   If the response should not include any metadata, then set this parameter
 *   to an empty array.
 *   Default: @code array(
 *     'pagination' => (object) NULL,
 *     'status' => (object) NULL,
 *     'datafiles' => array(),
 *   )@endcode.
 * @param array $debug_data
 *   An array/hash containing additional debugging information that should be
 *   output.
 *
 * @return string
 *   a PHP structure that will correspond to a JSON output.
 *
 * @ingroup brapi_api
 */
function brapi_json_prepare(
    $data = array(),
    $metadata = NULL,
    $debug_data = array()
  ) {

  $args = drupal_get_query_parameters();

  if (!isset($data)) {
    $data = array();
  }
  if (!isset($metadata)) {
    $metadata = brapi_prepare_metadata(0);
  }
  // Check if have metadata (either default or specified non-empty one).
  if (count($metadata)) {
    // Combine data and metadata.
    $output_data = array_merge($data, array('metadata' => $metadata));
  }
  else {
    // Only output data (no metadata).
    $output_data = $data;
  }

  // Display debug data in debug mode only.
  if (isset($args['debug']) && isset($debug_data)) {
    $output_data['debug'] = implode("\n", $debug_data);
  }

  return $output_data;
}

/**
 * Outputs the JSON data corresponding to a BRAPI call.
 *
 * Helper function that outputs the given PHP structure as a JSON element
 * through the HTTP(S) protocol.
 *
 * Supported GET parameters:
 * - pretty (bool)
 *   if set to a true value, output a nice indented human-readable JSON
 *   structure.
 *
 * @param mixed $output_data
 *   The PHP structure that represent the JSON data to serve.
 *
 * @ingroup brapi_api
 */
function brapi_json_output($output_data) {
  $args = drupal_get_query_parameters();

  if (!isset($args['pretty']) || $args['pretty']) {
    $output_data = json_encode(
      $output_data,
      JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES
    );
    drupal_json_output();
    echo $output_data;
  }
  else {
    drupal_json_output($output_data);
  }
}

/**
 * Returns an error response.
 *
 * Helper function that formats the given error response into a standardized
 * hash with all requested elements.
 *
 * @param int $http_error_code
 *   HTTP error code (usually 4xx or 5xx).
 * @param string $message
 *   Error description in one or more sentences. It should have already been
 *   processed by t().
 * @param array $data
 *   Response data hash if some.
 * @param array $debug_data
 *   An array/hash containing additional debugging information that should be
 *   output.
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
      ),
    ),
  );
  return brapi_json_prepare($data, $metadata, $debug_data);
}

/**
 * Returns an error response.
 *
 * Helper function that returns the given data structure as an error without
 * processing the structure to add any field except the debug one.
 *
 * Supported GET parameters:
 * - debug bool
 *   include debug information in output if some have been provided (see
 *   $debug_data parameter)
 *
 * @param int $http_error_code
 *   HTTP error code (usually 4xx or 5xx).
 * @param array $data
 *   Response data hash if some.
 * @param array $debug_data
 *   An array/hash containing additional debugging information that should be
 *   output.
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

  return $data;
}

/**
 * Tells if a given field of a given data type supports a given operation type.
 *
 * @param string $object_type
 *   Type of object/data. eg. 'germplasm'.
 * @param string $field
 *   Name of the field. eg. 'germplasmPUI'.
 * @param string $operation
 *   Type of operation to check. Possible values are 'create', 'read',
 *   'update', 'delete' and 'filter'.
 *
 * @return bool
 *   TRUE if the operation is supported and FALSE otherwise.
 */
function brapi_data_field_supports($object_type, $field, $operation) {
  $data_mapping = brapi_get_data_mapping();
  $field_fetcher = brapi_get_field_fetcher($data_mapping, $object_type, $field);
  if (!$field_fetcher) {
    return FALSE;
  }
  switch (gettype($field_fetcher)) {
    case 'string':
    case 'array':
      return TRUE;

    case 'object':
      if (is_callable($field_fetcher)) {
        // Check if the function supports filtering.
        $supported_ops = $field_fetcher($object_type, NULL, $field, NULL);
        if (!$supported_ops) {
          return FALSE;
        }
        return isset($supported_ops[$operation]) ?
          $supported_ops[$operation]
          : FALSE;
      }
      return FALSE;

    default:
      return FALSE;
  };

}

/**
 * Returns the list of parameters that can be used to filter call results.
 *
 * @param string $requested_call_name
 *   A BrAPI call name.
 *
 * @return array
 *   a hash with 2 keys, 'GET' and 'POST', containing hashes (associated to the
 *   corresponding method) with filter names as keys and data types as values.
 *
 * @ingroup brapi_api
 */
function brapi_get_call_filters($requested_call_name) {
  static $call_filters;

  if (!isset($call_filters)) {
    $call_filters = array();

    // Get all call settings and data mapping.
    $data_mapping = brapi_get_data_mapping();
    $call_settings = brapi_get_calls();

    // Loop on each call to get its parameters and filters.
    foreach ($call_settings as $call => $call_setting) {
      $call_filters[$call] = array();
      $versioned_parameters = isset($call_setting['parameters']) ?
        $call_setting['parameters']
        : array();
      $versioned_filters = isset($call_setting['filters']) ?
        $call_setting['filters']
        : array();

      $versions = array_keys(
        array_merge(
          $versioned_parameters,
          $versioned_filters
        )
      );
      sort($versions);

      // Loop on each call version.
      foreach ($versions as $api_version) {
        $call_filters[$call][$api_version] = array();

        if (isset($versioned_parameters[$api_version])) {
          $call_filters[$call][$api_version]['GET'] =
            $versioned_parameters[$api_version];
        }
        else {
          $call_filters[$call][$api_version]['GET'] = array();
        }

        if (isset($versioned_filters[$api_version])) {
          $filters = $versioned_filters[$api_version];
          $call_filters[$call][$api_version]['POST'] = array();

          // Loop on filtrable data types.
          foreach ($versioned_filters[$api_version] as $data_type => $field_names) {
            $data_type_fields = $data_mapping[$data_type]['fields'];
            $data_type_filtrable_fields = array();
            // Loop on each field.
            foreach ($field_names as $field_name) {
              if (!isset($data_type_fields[$field_name])) {
                drupal_set_message(
                  t(
                    'Warning: invalid filtering field @field_name for call @call (v@api_version)',
                    array(
                      '@field_name' => $field_name,
                      '@call' => $call,
                      '@api_version' => $api_version,
                    )
                  ),
                  'warning'
                );
                continue;
              }
              $field_fetcher = brapi_get_field_fetcher(
                $data_mapping,
                $data_type,
                $field_name
              );
              // Get widget type according to field type.
              switch (gettype($field_fetcher)) {
                case "string":
                  $data_type_filtrable_fields[$field_name] = 'string';
                  break;

                case "array":
                  switch ($field_fetcher['field_type']) {
                    case 'string':
                      $data_type_filtrable_fields[$field_name] = 'string';
                      break;

                    case 'int':
                      $data_type_filtrable_fields[$field_name] = 'int';
                      break;

                    case 'float':
                      $data_type_filtrable_fields[$field_name] = 'float';
                      break;

                    case 'bool':
                      $data_type_filtrable_fields[$field_name] = 'bool';
                      break;

                    case 'date':
                      $data_type_filtrable_fields[$field_name] = 'date';
                      break;

                    case 'cvterm':
                      $data_type_filtrable_fields[$field_name] =
                        "$data_type/$field_name";
                      break;

                    default:
                      break;
                  }
                  break;

                case "object":
                  break;

                default:
                  break;
              }
            }

            // Add data type fields to filtration.
            $call_filters[$call][$api_version]['POST'] +=
              $data_type_filtrable_fields;
          }
        }
        else {
          $call_filters[$call][$api_version]['POST'] = array();
        }
      }
    }
  }

  if (isset($call_filters[$requested_call_name])) {
    return $call_filters[$requested_call_name];
  }
  else {
    return array();
  }
}

/**
 * Aggregate call data.
 *
 * Aggregate data from other sites using a given call if the call aggregation
 * has been setup for current call.
 *
 * @param array $data
 *   Current call data. It will contain the aggregated data.
 * @param array $metadata
 *   Current call metadata.
 * @param array $debug_data
 *   Current call debug data.
 * @param array $parameters
 *   Call parameters. 'GET' key contains GET parameter hash and 'POST' key, POST
 *   parameter hash. Other keys are ignored.
 * @param string $call
 *   BrAPI call to use at remote BrAPI instance. Can be set to a FALSE value to
 *   have it guessed automatically. It may contain identifiers.
 *
 * @return int
 *   Returns the number of external site queried that successfully answered.
 *
 * @ingroup brapi_api
 */
function brapi_aggregate_call(
  &$data,
  &$metadata,
  &$debug_data,
  $parameters = array(),
  $call = NULL) {
  static $aggregation_options;
  // Get call aggregation settings.
  if (!isset($aggregation_options)) {
    $aggregation_options = variable_get(BRAPI_AGGREGATION_OPTIONS, array());
  }

  $queried_site_count = 0;

  // Get call menu used by Drupal.
  $menu_call_mapping = brapi_get_menu_call_mapping();
  $call_name = $menu_call_mapping['menu_to_call'][menu_get_item()['path']];

  // Set complete call path if omitted by caller. It will include parameters but
  // not query string.
  if (!$call) {
    $call = substr(current_path(), strlen(BRAPI_SERVICE_PREFIX_V1));
  }

  // Check parameters.
  if (!isset($parameters) ||!is_array($parameters)) {
    $parameters = array();
  }

  // Get the list of callers if some.
  $callers = array();
  if (isset($_GET['callers'])) {
    $caller_list = explode(',', $_GET['callers']);
    $callers = array_combine($caller_list, $caller_list);
  }
  // Add current site as a caller (to avoid infinite loops).
  global $base_url;
  // Remove protocol.
  $site_url = preg_replace('/^.*:\/\//', '', $base_url)
    . '/'
    . BRAPI_SERVICE_PREFIX_V1;
  $callers[$site_url] = $site_url;
  $parameters['GET']['callers'] = implode(',', $callers);

  // Check if call has aggregation.
  if (isset($aggregation_options[$call_name])
      && !empty($aggregation_options[$call_name])) {
    // Get data from other sites.
    $query = new EntityFieldQuery();
    $query->entityCondition('entity_type', 'brapi_site')
      ->propertyCondition('machine_name', $aggregation_options[$call_name])
      ->propertyOrderBy('weight', 'ASC');
    $results = $query->execute();
    if (!empty($results)) {
      // Load data about each connection.
      $brapi_sites = entity_load(
        'brapi_site',
        array_keys($results['brapi_site'])
      );

      $aggregated_from_list = array();
      foreach ($brapi_sites as $bsid => $brapi_site) {
        // Check if the site is in the list of caller URL to avoid infinite
        // loops. If so, go next.
        // Remove protocol.
        $site_url = preg_replace('/^.*:\/\//', '', $brapi_site->url);
        if (isset($callers[$site_url])) {
          continue;
        }
        $aggregated_from_list[] = $brapi_site->url;

        // Prepare URL.
        $url = $brapi_site->url
          . $call
          . (isset($parameters['GET']) ?
            '?' . http_build_query($parameters['GET'])
            : '');
        // Prepare context.
        // Check method to use.
        if (isset($parameters['POST']) && !empty($parameters['POST'])) {
          // POST.
          $json_post_data = json_encode($parameters['POST']);
          $options = array(
            'http' => array(
              'method'  => 'POST',
              'header'  => "Content-Type: application/json\r\n",
              'content' => $json_post_data,
            ),
          );
        }
        else {
          // GET.
          $options = array(
            'http' => array(
              'method'  => 'GET',
              'header'  => '',
            ),
          );
        }
        // @todo: Manage authentication.
        // If the site supports authentication, check if we got a valid
        // bearer. If not, get a new one and add it to the header.
        // @code
        // if ($username && $password) {
        //   ... array($brapi_site->username, $brapi_site->password);
        //   $options['http']['header'] .=
        //     'Authorization: Bearer '
        //     . base64_encode($bearer)
        //     . "\r\n";
        // }
        // @endcode
        try {
          $context = stream_context_create($options);
          if (!$call_result_json = file_get_contents($url, FALSE, $context)) {
            // If it failed, try without context.
            // $call_result_json = file_get_contents($url);
          }
          if (isset($call_result_json)) {
            $call_result = json_decode($call_result_json, TRUE);
            if (isset($call_result)) {
              // Count the answer.
              ++$queried_site_count;
              $data = brapi_merge_array($data, $call_result);
              // @todo +FIXME: take into account metadata (pager, files).
              // @code unset($data['metadata']); @endcode
            }
          }
        }
        catch (Exception $error_handle) {
          drupal_set_message(
            t(
              "ERROR calling '@url': @message",
              array(
                '@url' => $url,
                '@message' => $error_handle->getMessage(),
              )
            ),
            'error'
          );
        }
      }
      // Add to metadata the list of caller URLs.
      foreach ($aggregated_from_list as $remote_url) {
        $metadata['status'][] = array(
          'messageType' => 'INFO',
          'message' => 'aggregated_from: ' . $remote_url,
        );
      }
    }
  }

  return $queried_site_count;
}

/**
 * Merge two arrays recursively.
 *
 * Merge content of $b that is not into $a to $a and returns the merging result.
 * If a (sub)key is present in both arrays and its value is an array, the 2
 * arrays are merged recursively using brapi_merge_array. If the value is not
 * set in $a, the value of $b is used.
 *
 * @code
 * $a = array('a' => 1, 'b' => array('x' => 5, 'y' => 7), 'c' => 6, 'e' => NULL);
 * $b = array('a' => 2, 'b' => array('x' => 3, 'z' => 4), 'd' => 8, 'e' => 9);
 * $c = brapi_merge_array($a, $b);
 * // $c will be:
 * array(
 *   'a' => 1,
 *   'b' => array('x' => 5, 'y' => 7, 'z' => 4),
 *   'c' => 6,
 *   'd' => 8,
 *   'e' => 9,
 * );
 * @endcode
 *
 * @param array $a
 *   First array.
 * @param array $b
 *   Second array.
 *
 * @return array
 *   Merged array.
 */
function brapi_merge_array($a, $b) {
  if (isset($a[0])) {
    // Merge sequences.
    $a = array_merge($a, $b);
  }
  else {
    // Merge common keys on associative array.
    foreach ($a as $key => $value) {
      if (isset($b[$key])) {
        if (is_array($a[$key])) {
          if (is_array($b[$key])) {
            $a[$key] = brapi_merge_array($value, $b[$key]);
          }
        }
        elseif (empty($a[$key])) {
          $a[$key] = $b[$key];
        }
      }
    }
    // Add b-specific keys to a.
    $a += $b;
  }

  // Returns merged array.
  return $a;
}

/**
 * Transcribes a BrAPI wildcarded search string into a PostgreSQL search string.
 */
function brapi_transcribe_search_string_to_postgres(
  $string,
  $similar_to_syntax = FALSE
) {
  if ($similar_to_syntax) {
    // Escape special characters for SIMILAR TO.
    $string = str_replace(
      array('|', '+', '(', ')', '[', ']', '{', '}'),
      array('\|', '\+', '\(', '\)', '\[', '\]', '\{', '\}'),
      $string
    );
  }
  // Replaces non-escaped '*' with '%' and escape '_'.
  $string = preg_replace(
    array(
      '/(^|[^\\\\])\*/',
      '/_/',
    ),
    array(
      '${1}%',
      '\_',
    ),
    $string
  );
  // Replaces non-escaped '?' with '_'.
  $string = preg_replace(
    // Do it twice to replace following '?' which would be skipped
    // otherwise.
    array('/(^|[^\\\\])\?/', '/(^|[^\\\\])\?/'),
    '${1}_',
    $string
  );
  // Unescape ILIKE non-special char.
  if (!$similar_to_syntax) {
    $string = str_replace(array('\?', '\*'), array('?', '*'), $string);
  }

  return $string;
}

/**
 * Adds given fields to the non-exact search list and return that list.
 *
 * @param array $fields
 *   An associative array which keys are field names and values are associative
 *   arrays with 3 keys: 'table' (name of the Chado table), 'column' (name of
 *   the Chado table column holding the string value) and 'match' (one of the
 *   following matching methods: 'case_insensitive', 'wildcard').
 *
 * @return array
 *   An associative array which keys are field names and values are associative
 *   arrays with 3 keys: 'table' (name of the Chado table), 'column' (name of
 *   the Chado table column holding the string value) and 'match' (one of the
 *   following matching methods: 'case_insensitive', 'wildcard').
 */
function brapi_non_exact_search($fields = FALSE) {
  static $non_exact_fields;

  if (!isset($non_exact_fields)) {
    $non_exact_fields = array();
  }

  if ($fields) {
    $non_exact_fields = array_merge($non_exact_fields, $fields);
  }

  return $non_exact_fields;
}

/**
 * Forces the pager settings to bypass Tripal 2 paging issue.
 *
 * @param array $pager
 *   an array with 2 keys: 'limit' and 'element'. If not set or empty, the last
 *   given pager value is kept.
 *
 * @return array
 *   the last $pager value given.
 */
function brapi_force_pager($pager = FALSE) {
  static $forced_pager;
  if ($pager) {
    $forced_pager = $pager;
  }
  return $forced_pager;
}

/**
 * Example implementation of hook_brapi_CALL_FUNC_NAME_alter.
 *
 * This implementation adds a source_db field set to 'brapi' to any call result
 * data.
 *
 * <CALL_FUNC_NAME> should be replaced by the basic call path with non-word
 * characters being replaced by underscores and all lower-cased. Available call
 * path can be found in brapi.const.inc "brapi_get_calls()" function: keys of
 * the returned array are the actual call path.
 * Ex.:
 * * call 'germplasm-search' --> MY_MODULE_brapi_germplasm_search_alter
 * * call 'germplasm/{germplasmDbId}' -->
 *   MY_MODULE_brapi_germplasm_germplasmdbid_alter
 *
 * @param array $data
 *   The $data variable contains the structure of the result part of the call
 *   result.
 * @param array $context
 *   BrAPI 'metadata' and 'debug' structures can be accessed and modified
 *   through $context['metadata'] and $context['debug'] fields.
 *
 * @see brapi_get_calls()
 */
function hook_brapi_CALL_FUNC_NAME_alter(&$data, &$context) {
  $data['result']['data']['source_db'] = 'brapi';
}

/**
 * Example implementation of hook_brapi_CALL_FUNC_NAME_brapi_error_alter.
 *
 * Allows other modules to alter error raised by BrAPI in any BrAPI call.
 *
 * <CALL_FUNC_NAME> should be replaced by the basic call path with non-word
 * characters being replaced by underscores. Available call path can be found
 * in brapi.const.inc "brapi_get_calls()" function: keys of the returned array
 * are the actual call path.
 * Ex.:
 * * call 'germplasm-search' -->
 *   MY_MODULE_brapi_germplasm_search_brapi_error_alter
 * * call 'germplasm/{germplasmDbId}' -->
 *   MY_MODULE_brapi_germplasm_germplasmdbid_brapi_error_alter
 *
 * @param string $message
 *   contains the error content displayed to the user. To change
 *   the status code returned, implementation should use
 *   drupal_add_http_header('Status', $http_error_code).
 *
 * @see brapi_get_calls()
 */
function hook_brapi_CALL_FUNC_NAME_brapi_error_alter(&$message) {
  $message = '<div>It seems a BrAPI error occured!</div>';
}

/**
 * Example implementation of hook_brapi_CALL_FUNC_NAME_error_alter.
 *
 * Allows other modules to alter any non-BrAPI error in any BrAPI call.
 *
 * <CALL_FUNC_NAME> should be replaced by the basic call path with non-word
 * characters being replaced by underscores. Available call path can be found
 * in brapi.const.inc "brapi_get_calls()" function: keys of the returned array
 * are the actual call path.
 * Ex.:
 * * call 'germplasm-search' --> MY_MODULE_brapi_germplasm_search_error_alter
 * * call 'germplasm/{germplasmDbId}' -->
 *   MY_MODULE_brapi_germplasm_germplasmdbid_error_alter
 *
 * @param string $message
 *   contains the error content displayed to the user. To change
 *   the status code returned, implementation should use
 *   drupal_add_http_header('Status', $http_error_code).
 *
 * @see brapi_get_calls()
 */
function hook_brapi_CALL_FUNC_NAME_error_alter(&$message) {
  $message = '<div>It seems an error occured!</div>';
}

/**
 * Returns a hash containing BrAPI calls and Drupal menu mapping.
 *
 * @param bool $reset
 *   If true, reset the cache and re-compute the mapping.
 *
 * @return array
 *   Returns a hash with 2 entries, 'call_to_menu' and 'menu_to_call' that both
 *   contain a hash that associate to a given entry in a system (Drupal menu or
 *   BrAPI call) its corresponding entry in the other system.
 */
function brapi_get_menu_call_mapping($reset = FALSE) {
  static $menu_call_mapping;

  if (!isset($menu_call_mapping) || $reset) {
    if (!$reset
        && ($cache = cache_get('BRAPI_MENU_CALL_MAPPING'))
        && !empty($cache->data)) {
      $menu_call_mapping = $cache->data;
    }
    else {
      $menu_call_mapping = array(
        'call_to_menu' => array(),
        'menu_to_call' => array(),
      );
      foreach (brapi_get_calls() as $call_name => $call_data) {
        $call_url = BRAPI_SERVICE_PREFIX_V1 . '/' . $call_name;
        // Remove trailing slash if one.
        if ('/' == $call_url[strlen($call_url) - 1]) {
          $call_url = substr($call_url, 0, -1);
        }
        // Replace place holders.
        $call_url = preg_replace('/\{\w+\}/', '%', $call_url);
        $menu_call_mapping['call_to_menu'][$call_name] = $call_url;
        $menu_call_mapping['menu_to_call'][$call_url] = $call_name;
      }

      cache_set('BRAPI_MENU_CALL_MAPPING', $menu_call_mapping);
    }
  }

  return $menu_call_mapping;
}

/**
 * Returns the query interface form for a given call.
 *
 * @param object $form
 *   Drupal form. See Form API.
 * @param object $form_state
 *   Drupal form state. See Form API.
 * @param string $call_name
 *   BrAPI call name.
 * @param array $call_info
 *   Call information structure slightly different from the one returned by
 *   brapi_get_calls().
 *   Ex.: @code array(
 *     'arguments' => array(
 *       3 => array(
 *         'name' => 'someTypeDbId',
 *         'type' => 'string',
 *         'description' => 'Some argument description',
 *         'required' => 1,
 *       ),
 *     ),
 *     'parameters-filters' => array(
 *       '1.0' => array(
 *         'GET' => array(
 *           'matchMethod' => array('exact', 'case_insensitive', 'wildcard'),
 *           ...
 *         ),
 *         'POST' => array(
 *           filterParamName1 => string,
 *           filterParamName2 => germplasm/typeOfGermplasmStorageCode,
 *           filterParamName3 => date,
 *           ...
 *         ),
 *       ),
 *       '1.1' => ...
 *       ...
 *     ),
 *     'api versions' => array('1.0', '1.1', ...),
 *   ); @endcode.
 *
 * @return array
 *   The renderable form array.
 */
function brapi_query_form($form, &$form_state, $call_name, $call_info) {
  // Call action URL.
  global $base_url;
  $call_url = url(BRAPI_SERVICE_PREFIX_V1 . '/' . $call_name);
  $version_classes = array_map(
    function ($version) {
      $version_mn = preg_replace('/\W+/', '_', $version);
      return 'brapi-query-api-' . $version_mn;
    },
    $call_info['api versions']
  );
  $form['#action'] = $call_url;
  $form['#attributes'] = array(
    'target' => 'brapi_query_result_iframe',
    'class' => array_merge(array('brapi-query-call'), $version_classes),
  );
  $form['call_name'] = array(
    '#markup' => '<span class="hidden brapi-hidden" name="call_name">' . $call_name . '</span>',
  );

  // Call name/machine name.
  $call_machine_name = preg_replace('/\W+/', '_', $call_name);
  if ('' == $call_name) {
    $call_title = t('Root call');
  }
  else {
    $call_title = t(
      'Call @call_name',
      array('@call_name' => $call_name)
    );
  }

  // Main call fieldset.
  $call_fieldset = array(
    '#type'        => 'fieldset',
    '#title'       => $call_title,
  );
  $form['call_' . $call_machine_name] = &$call_fieldset;

  // Call arguments.
  if ($call_info['arguments']) {
    $call_fieldset['arguments'] = array(
      '#type'        => 'fieldset',
      '#title'       => t('Arguments:'),
    );
    foreach ($call_info['arguments'] as $arg_number => $argument) {
      $call_fieldset['arguments'][$argument['name']] = array(
        '#type' => 'textfield',
        '#title' => $argument['name'],
        '#required' => $argument['required'],
        '#size' => 40,
        '#attributes' => array(
          'class' => array('brapi-query-argument'),
        ),
      );
    }
  }

  // Call filters.
  if ($call_info['parameters-filters']) {
    // Loop on API versions.
    foreach ($call_info['parameters-filters'] as $api_version => $call_methods) {
      $api_version_mn = preg_replace('/\W+/', '_', $api_version);
      $filter_fieldset = array(
        '#type'  => 'fieldset',
        '#title' => t('v@version filters:', array('@version' => $api_version)),
        '#collapsible' => TRUE,
        '#collapsed'   => FALSE,
        '#attributes' => array(
          'class' => array('brapi-query-filters', 'brapi-query-api-' . $api_version_mn),
        ),
      );
      $call_fieldset[$call_machine_name . '-' . $api_version_mn . '-filters'] = &$filter_fieldset;
      // Loop on methods.
      foreach (array('GET', 'POST') as $method) {
        foreach ($call_methods[$method] as $filter_name => $filter_type) {
          if (is_array($filter_type)) {
            $options = array(
              '' => t('Not set'),
            )
            + array_combine($filter_type, $filter_type);

            $filter_fieldset[$filter_name] = array(
              '#type' => 'select',
              '#options' => $options,
              '#default_value' => '',
              '#title' => $filter_name,
              '#attributes' => array(
                'class' => array('brapi-query-filter-' . strtolower($method)),
              ),
            );
          }
          elseif (('string' == $filter_type)
                  || ('int' == $filter_type)
                  || ('float' == $filter_type)) {
            $filter_fieldset[$filter_name] = array(
              '#type' => 'textfield',
              '#title' => $filter_name,
              '#default_value' => '',
              '#size' => 80,
              '#maxlength' => 1024,
              '#attributes' => array(
                'class' => array('brapi-query-filter-' . strtolower($method)),
              ),
            );
          }
          elseif ('bool' == $filter_type) {
            $filter_fieldset[$filter_name] = array(
              '#type' => 'select',
              '#title' => $filter_name,
              '#options' => array(
                'TRUE' => 1,
                'FALSE' => 0,
              ),
              '#default_value' => 0,
              '#multiple' => FALSE,
              '#attributes' => array(
                'class' => array('brapi-query-filter-' . strtolower($method)),
              ),
            );
          }
          elseif ('date' == $filter_type) {
            $filter_fieldset[$filter_name] = array(
              '#type' => 'textfield',
              '#title' => $filter_name,
              '#default_value' => '',
              '#size' => 10,
              '#maxlength' => 10,
              '#attributes' => array(
                'class' => array(
                  'brapi-datepicker',
                  'brapi-query-filter-' . strtolower($method),
                ),
              ),
            );
          }
          elseif (FALSE !== strpos($filter_type, '/')) {
            $filter_fieldset[$filter_name] = array(
              '#type' => 'textfield',
              '#title' => $filter_name,
              '#default_value' => '',
              '#autocomplete_path' => 'brapi/filter/autocomplete/' . $filter_type,
              '#size' => 80,
              '#maxlength' => 1024,
              '#attributes' => array(
                'class' => array('brapi-query-filter-' . strtolower($method)),
              ),
            );
          }
        }
      }
      // Release array to instantiate a new one on next loop.
      unset($filter_fieldset);
    }
  }

  $call_fieldset['global'] = array(
    '#type'  => 'fieldset',
    '#title' => t('Settings'),
    '#attributes' => array(
      'class' => array('brapi-query-parameters'),
    ),

    'page' => array(
      '#type' => 'textfield',
      '#title' => t('Page number'),
      '#default_value' => '0',
      '#size' => 10,
      '#maxlength' => 10,
    ),

    'pageSize' => array(
      '#type' => 'textfield',
      '#title' => t('Results per page'),
      '#default_value' => '0',
      '#size' => 10,
      '#maxlength' => 10,
    ),

    'pretty' => array(
      '#type' => 'checkbox',
      '#title' => t('Output pretty JSON'),
      '#return_value' => '1',
      '#default_value' => TRUE,
    ),

  );

  if (user_access(BRAPI_ADMIN_PERMISSION)) {
    $call_fieldset['global']['debug'] = array(
      '#type' => 'checkbox',
      '#title' => t('Enable debug mode'),
      '#return_value' => '1',
      '#default_value' => TRUE,
    );
  }

  $call_fieldset['submit'] = array(
    '#type'        => 'submit',
    '#value'       => t('Submit'),
  );

  return $form;
}

/**
 * Returns information about an organism.
 *
 * @param int $organism_id
 *   Value of organism.organism_id of the organism of interest.
 *
 * @return array
 *   An array with the following keys:
 *   - "species": name of the species/group or NULL;
 *   - "speciesAuthority": species authority or NULL;
 *   - "subtaxa": name of the subspecies or subgroup or NULL;
 *   - "subtaxaAuthority": subtaxa authority or NULL.
 */
function brapi_get_organism_info($organism_id) {
  static $organism_data = array();
  $cv_settings = brapi_get_cv_settings();

  if (!array_key_exists($organism_id, $organism_data)) {
    $sql_query = '
      SELECT
        so.species AS "species",
        sop.value AS "speciesAuthority",
        sso.species AS "subtaxa",
        ssop.value AS "subtaxaAuthority"
      FROM {organism} o
      LEFT JOIN {phylonode_organism} po ON (po.organism_id = o.organism_id)
      LEFT JOIN {phylonode} pn ON (po.phylonode_id = pn.phylonode_id)
      LEFT JOIN {phylonode} spn ON (
        spn.left_idx <= pn.left_idx
        AND spn.right_idx >= pn.right_idx
        AND spn.type_id IN (:species)
      )
      LEFT JOIN {phylonode_organism} spno ON (spno.phylonode_id = spn.phylonode_id)
      LEFT JOIN {organism} so ON (spno.organism_id = so.organism_id)
      LEFT JOIN {organismprop} sop ON (
        spno.organism_id = sop.organism_id
        AND sop.type_id IN (:species_authority)
      )
      LEFT JOIN {phylonode} sspn ON (
        sspn.left_idx <= pn.left_idx
        AND sspn.right_idx >= pn.right_idx
        AND sspn.type_id IN (:subtaxa)
      )
      LEFT JOIN {phylonode_organism} sspno ON (sspno.phylonode_id = sspn.phylonode_id)
      LEFT JOIN {organism} sso ON (sspno.organism_id = sso.organism_id)
      LEFT JOIN {organismprop} ssop ON (
        sspno.organism_id = ssop.organism_id
        AND ssop.type_id IN (:subtaxa_authority)
      )
      WHERE
        o.organism_id = :organism_id
      ;';
    $filter_values = array(
      ':organism_id' => $organism_id,
      ':species' => $cv_settings['species'],
      ':species_authority' => $cv_settings['speciesAuthority'],
      ':subtaxa' => $cv_settings['subtaxa'],
      ':subtaxa_authority' => $cv_settings['subtaxaAuthority'],
    );
    $species_result = chado_query($sql_query, $filter_values);
    if ($species_result) {
      $organism_data[$organism_id] = $species_result->fetchAssoc();
    }
    else {
      $organism_data[$organism_id] = array(
        'species' => NULL,
        'speciesAuthority' => NULL,
        'subtaxa' => NULL,
        'subtaxaAuthority' => NULL,
      );
    }
  }
  return $organism_data[$organism_id];
}

/**
 * Returns information about an organism.
 * This version uses the chado 1.3 type_id and infraspecific_name columns
 * in the organism table to generate the subtaxa name.
 *
 * @param int $organism_id
 *   Value of organism.organism_id of the organism of interest.
 *
 * @return array
 *   An array with the following keys:
 *   - "species": name of the species/group or NULL;
 *   - "speciesAuthority": species authority or NULL;
 *   - "subtaxa": name of the subspecies or subgroup or NULL;
 *   - "subtaxaAuthority": subtaxa authority or NULL.
 */
function brapi_get_subtaxon_info($organism_id) {
  static $organism_data = array();
  $cv_settings = brapi_get_cv_settings();

  if (!array_key_exists($organism_id, $organism_data)) {
    $sql_query = '
      SELECT
        o.species AS "species",
        sop.value AS "speciesAuthority",
        CONCAT_WS(\' \', REPLACE(cv.name, \'no_rank\', \'\'), o.infraspecific_name) AS "subtaxa",
        ssop.value AS "subtaxaAuthority"
      FROM {organism} o
      LEFT JOIN {cvterm} cv ON (
        o.type_id = cv.cvterm_id
      )
      LEFT JOIN {organismprop} sop ON (
        o.organism_id = sop.organism_id
        AND sop.type_id IN (:species_authority)
      )
      LEFT JOIN {organismprop} ssop ON (
        o.organism_id = ssop.organism_id
        AND ssop.type_id IN (:subtaxa_authority)
      )
      WHERE
        o.organism_id = :organism_id
      ;';
    $filter_values = array(
      ':organism_id' => $organism_id,
      ':species_authority' => $cv_settings['speciesAuthority'],
      ':subtaxa_authority' => $cv_settings['subtaxaAuthority'],
    );
    $species_result = chado_query($sql_query, $filter_values);
    if ($species_result) {
      $organism_data[$organism_id] = $species_result->fetchAssoc();
    }
    else {
      $organism_data[$organism_id] = array(
        'species' => NULL,
        'speciesAuthority' => NULL,
        'subtaxa' => NULL,
        'subtaxaAuthority' => NULL,
      );
    }
  }
  return $organism_data[$organism_id];
}

/**
 * Helper function that returns CV/CV term filter for CV terms.
 *
 * @param array $cvterm_settings
 *   An array containing the CV term selection settings.
 *
 * @return array
 *   2 elements: the first one is the WHERE clause, and the second one is the
 *   associated values.
 */
function brapi_v1_get_cvterm_filter($cvterm_settings) {

  // Get all terms for current term settings.
  $cvterm_ids = array();
  $parent_cvterm_ids = array();
  $cv_ids = array();
  // If we got a list of cvterms.
  if ($cvterm_settings['cvterms']) {
    $cvterm_refs = explode(',', $cvterm_settings['cvterms']);
    foreach ($cvterm_refs as $cvterm_ref) {
      if (preg_match('/(\d+)\D*$/', $cvterm_ref, $matches)) {
        $cvterm_ids[$matches[1]] = TRUE;
      }
    }
  }
  // Get from relationship.
  if ($cvterm_settings['parent_cvterms']) {
    $cvterm_refs = explode(',', $cvterm_settings['parent_cvterms']);
    foreach ($cvterm_refs as $cvterm_ref) {
      if (preg_match('/(\d+)\D*$/', $cvterm_ref, $matches)) {
        $parent_cvterm_ids[$matches[1]] = TRUE;
      }
    }
  }
  // Get terms from CV.
  if ($cvterm_settings['cvs']) {
    $cv_refs = explode(',', $cvterm_settings['cvs']);
    foreach ($cv_refs as $cv_ref) {
      if (preg_match('/(\d+)\D*$/', $cv_ref, $matches)) {
        $cv_ids[$matches[1]] = TRUE;
      }
    }
  }

  $filter_values = array(
    ':cvterm_ids' => array_keys($cvterm_ids),
    ':parent_cvterm_ids' => array_keys($parent_cvterm_ids),
    ':cv_ids' => array_keys($cv_ids),
  );

  $sql_filter_list = array();
  if (count($cvterm_ids)) {
    $sql_filter_list[] = 'cvt.cvterm_id IN (:cvterm_ids)';
  }

  if (count($parent_cvterm_ids)) {
    $sql_filter_list[] = 'cvt.cvterm_id IN (
      SELECT child_cvtr.subject_id
      FROM {cvterm_relationship} child_cvtr
        JOIN {cvterm} child_cvtr_type
          ON (child_cvtr.type_id = child_cvtr_type.cvterm_id)
      WHERE child_cvtr.object_id IN (:parent_cvterm_ids)
        AND child_cvtr_type.name IN (\'is_a\', \'isa\')
    )';
  }

  if (count($cv_ids)) {
    $sql_filter_list[] = 'cvt.cv_id IN (:cv_ids)';
  }

  $sql_filter = implode(' OR ', $sql_filter_list);

  return array($sql_filter, $filter_values);
}

/**
 * Helper function that returns CV/CV term selector for CV terms.
 *
 * @param array $cvterm_settings
 *   An array containing the CV term selection settings.
 *
 * @return array
 *   Returns a field selector that can be used by Tripal functions like
 *   chado_select_record() to select cvterms from the cvterm table.
 *
 * @link http://api.tripal.info/api/tripal/tripal_core!api!tripal_core.chado_query.api.inc/function/chado_select_record/2.x chado_select_record() @endlink
 */
function brapi_v1_get_cvterm_selector($cvterm_settings) {

  // Get all terms for current term settings.
  $selector = array();
  // If we got a list of cvterms.
  if ($cvterm_settings['cvterms']) {
    $cvterm_refs = explode(',', $cvterm_settings['cvterms']);
    foreach ($cvterm_refs as $cvterm_ref) {
      if (preg_match('/(\d+)\D*$/', $cvterm_ref, $matches)) {
        $cvterm_ids[$matches[1]] = TRUE;
      }
    }
    if (!empty($cvterm_ids)) {
      $selector['cvterm_id'] = array_keys($cvterm_ids);
    }
  }
  // Get from relationship.
  if ($cvterm_settings['parent_cvterms']) {
    $cvterm_refs = explode(',', $cvterm_settings['parent_cvterms']);
    foreach ($cvterm_refs as $cvterm_ref) {
      if (preg_match('/(\d+)\D*$/', $cvterm_ref, $matches)) {
        $parent_cvterm_ids[$matches[1]] = TRUE;
      }
    }

    if (!empty($parent_cvterm_ids)) {
      $sql_query = "
        SELECT DISTINCT subject_id
        FROM cvterm_relationship
          JOIN cvterm ON (cvterm_id = type_id AND name IN ('is_a', 'isa'))
        WHERE
          object_id IN (:values)
      ;";
      $results = chado_query(
        $sql_query,
        array(':values' => array_keys($parent_cvterm_ids))
      )->fetchCol();

      if (array_key_exists('cvterm_id', $selector)) {
        $selector['cvterm_id'] = array_merge(
          $selector['cvterm_id'],
          $results
        );
      }
      else {
        $selector['cvterm_id'] = $results;
      }
    }
  }

  // Get terms from CV.
  if ($cvterm_settings['cvs']) {
    $cv_refs = explode(',', $cvterm_settings['cvs']);
    foreach ($cv_refs as $cv_ref) {
      if (preg_match('/(\d+)\D*$/', $cv_ref, $matches)) {
        $cv_ids[$matches[1]] = TRUE;
      }
    }
    if (!empty($cv_ids)) {
      $selector['cv_id'] = array_keys($cv_ids);
    }
  }

  return $selector;
}
