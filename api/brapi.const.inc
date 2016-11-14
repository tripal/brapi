<?php

/**
 * @file
 * Defines Breeding API Drupal module constants.
 *
 * @ingroup brapi
 */

/**
 * @defgroup brapi_const Breeding API global constants
 * @ingroup brapi
 * @{
 * Constants and default settings used by the Breeding API module.
 * @}
 */

/*******************************************************************************
 * BREEDING API GLOBALS
 *******************************************************************************
 */
define('BRAPI_SERVICE_VERSION', 'v1.0');
define('BRAPI_SERVICE_PREFIX_V1', 'brapi/v1');
define('BRAPI_MCPD_VERSION', '2.1');
define('BRAPI_DB', 'breeding api');
define('BRAPI_CV', 'breeding api');
define('BRAPI_CV_SETTINGS', 'brapi_cv_settings');
define('BRAPI_EXAMPLE_SETTINGS', 'brapi_example_settings');
define('BRAPI_USE_PERMISSION', 'use breeding api');
define('BRAPI_UPDATE_PERMISSION', 'update through breeding api');
define('BRAPI_ADMIN_PERMISSION', 'administer breeding api');
define('BRAPI_MULTICROP_PASSPORT_ONTOLOGY_CV', 'multicrop passport ontology');
define('BRAPI_DEFAULT_PAGE_SIZE', 30);
define('BRAPI_ESCAPE_CHARACTER', "\\");

/**
 * Returns an array of CV terms used by the Breeding API that are not part of
 * the MCPD ontology.
 *
 * @return array
 *   Keys are CV term names and values are the associated descriptions.
 *
 * @see brapi_get_mcpd_settings()
 *
 * @ingroup brapi_const
 */
function brapi_get_cv() {
  return array(
    'FemaleParentOf' => 'Female parent relationship (subject stock is the female parent of the object stock)',
    'MaleParentOf'   => 'Male parent relationship (subject stock is the male parent of the object stock)',
    'SeedSource'     => 'Name or identifier of the seed provider',
    'Germplasm'      => 'Germplasm type (default stock.type_id)',
  );
}

/**
 * Returns an array of supported Breeding API calls.
 *
 * @return array
 *   First level keys are call names including a slash prefix but not the
 *   "/brapi/v1" prefix. For instance, the germplasm call "/brapi/v1/germplasm"
 *   will have the name "/germplasm".
 *   Each call is described by an array with the keys:
 *   - 'title' (string): title of the call displayed on the user interface;
 *   - 'description' (string): description of the call;
 *   - 'datatypes' (array): array of data format (as strings) supported by the
 *     call;
 *   - 'methods' (array): array of HTTP methods as keys and their associated
 *     permission as values;
 *   - 'page callback': name of the function that returns the JSON output;
 *   - 'arguments' (array): an array where indices are the call URL argument
 *     position and values are arrays describing the argument:
 *     - 'name' (string): name of the argument;
 *     - 'type' (string): type of the argument;
 *     - 'description' (string): description of the argument;
 *     - 'required' (bool): true if the argument is required.
 *   - 'features' (array): an array of feature name => value/description
 *     supported by the call.
 *
 * @ingroup brapi_const
 */
function brapi_get_calls() {
  static $brapi_calls;
  if (!isset($brapi_calls)) {
    $brapi_calls = array(
      '/'               => array(
        'title'          => 'About',
        'description'    => 'About Breeding API Drupal implementation',
        'datatypes'      => array(),
        'methods'        => array(
          'GET' => 'access content',
        ),
        'page callback'  => 'brapi_v1_about_json',
      ),
      '/token'      => array(
        'title'          => 'Authentication',
        'description'    => 'User authentication in OAuth2 style',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'    => BRAPI_USE_PERMISSION,
          'POST'   => BRAPI_USE_PERMISSION,
          'DELETE' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => 'brapi_v1_authentication_json',
      ),
      '/calls'      => array(
        'title'          => 'Call search',
        'description'    => 'Supported Breeding API calls',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => 'brapi_v1_call_json',
        'arguments'      => array(
          3 => array(
            'name' => 'callName',
            'type' => 'string',
            'description' => 'a call name (machine name)',
            'required' => FALSE,
          ),
        ),
      ),
      '/germplasm-search' => array(
        'title'          => 'Germplasm Search',
        'description'    => 'Germplasm search call',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_UPDATE_PERMISSION,
        ),
        'page callback'  => 'brapi_v1_germplasm_search_json',
        'features'       => array('MCPD' => 'yes', 'MCPD-version' => 'V.' . BRAPI_MCPD_VERSION),
      ),
      '/germplasm'      => array(
        'title'          => 'Germplasm Details',
        'description'    => 'Germplasm Details by germplasmDbId',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'    => BRAPI_USE_PERMISSION,
          'POST'   => BRAPI_UPDATE_PERMISSION,
          'PUT'    => BRAPI_UPDATE_PERMISSION,
          'DELETE' => BRAPI_UPDATE_PERMISSION,
        ),
        'page callback'  => 'brapi_v1_germplasm_json',
        'arguments'      => array(
          3 => array(
            'name' => 'germplasmDbId',
            'type' => 'string',
            'description' => 'correspond to a Chado stock.uniquename value',
            'required' => FALSE,
          ),
        ),
        'features'       => array('MCPD' => 'yes', 'MCPD-version' => 'V.' . BRAPI_MCPD_VERSION),
      ),
    );
  }
  return $brapi_calls;
}

/**
 * Returns Breeding API data mapping.
 *
 * Returns Breeding API data mapping which is a structure describing the data
 * mapping between BrAPI data and Tripal Chado data.
 *
 * @return array
 *   Each entry of the hash
 *   returned is a BrAPI data type like 'germplasm' or 'study'. Each entry is
 *   composed by the following fields:
 *   - 'table': name of the main Chado table holding the record;
 *   - 'identifier': the Chado database key used to identify the record;
 *   - 'accession': the Chado database field used to identify the record from
 *     outside (Restful, web, apps);
 *   - 'fields': an array of field configuration;
 *
 *   Each field can have the following structures:
 *   - a string: it will be treated as the name of the column holding the value
 *     in the Chado table;
 *   - a hash structure: it configures how to fetch the field for Chado:
 *      - table: the name of the table holding the field;
 *      - object_key: name of the object field that must be used to match
 *        records in the other table. If not set, the 'identifier'
 *        field value of the data type field is used;
 *      - foreign_key: name of the field in the other table that should be used
 *        to match either the 'object_key' if specified or the 'identifier';
 *      - column: the name of the column holding the field value;
 *      - selector: a Tripal selector as described here as '$values':
 *        http://api.tripal.info/api/tripal/tripal_core%21api%21tripal_core.chado_query.api.inc/group/tripal_chado_query_api/2.x
 *      - field_type: one of 'string', 'int', 'bool' and 'cvterm' (cvterms
 *        are cvterm_id turned into cvterm.name);
 *      - return_array: tells if an array of values should be returned;
 *      - use_ranking: tells if the field table has a rank column to use;
 *      - fallback: another field name (or ordered array of field names) that can
 *        be used to fetch a value if none has been found with current field;
 *   - a closure: the closure is run to fetch or store the field value with the
 *     following arguments:
 *      - $data_type: the type of record in the BrAPI terminology (germplasm,
 *        study, ...);
 *      - $object: the corresponding Chado object loaded from database;
 *      - $field_name: the name of the record field to work on;
 *      - $op: the operation to perform. Can be one of 'create', 'read', 'update',
 *        'delete'. If not provided, 'read' is assumed;
 *      - $value (not provided for 'read' $op and optional for 'delete' $op): the
 *        new field value (or value to delete in case of 'delete' $op).
 *
 * @ingroup brapi_const
 */
function brapi_get_data_mapping() {
  static $brapi_data_mapping;

  $cv_settings = brapi_get_cv_settings();

  if (!isset($brapi_data_mapping)) {
    $brapi_data_mapping = array(
      'germplasm'      => array(
        'table' => 'stock',
        'identifier' => 'stock_id',
        'accession' => 'uniquename',
        'fields' => array(
          'germplasmDbId' => 'uniquename',
          'defaultDisplayName' => 'name',
          'accessionNumber' => array(
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['accessionNumber'],
            ),
            'field_type' => 'string',
          ),
          'germplasmName' => array(
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['germplasmName'],
            ),
            'field_type' => 'string',
          ),
          'germplasmPUI' => function ($data_type, $stock, $field_name) {
            global $base_url;
            if ($nid = chado_get_nid_from_id('stock', $stock->stock_id)) {
              return $base_url . '/' . drupal_get_path_alias('node/' . $nid);
            }
            else {
              return '';
            }
          },
          'femaleParentStockId' => array(
            'table' => 'stock_relationship',
            'column' => 'subject_id',
            'foreign_key' => 'object_id',
            'selector' => array(
              'type_id' => $cv_settings['FemaleParentOf'],
            ),
            'field_type' => 'int',
          ),
          'maleParentStockId' => array(
            'table' => 'stock_relationship',
            'column' => 'subject_id',
            'foreign_key' => 'object_id',
            'selector' => array(
              'type_id' => $cv_settings['MaleParentOf'],
            ),
            'field_type' => 'int',
          ),
          'pedigree' => function ($data_type, $stock, $field_name, $op) {
            $pedigree = '';
            $female_parent_stock_id = brapi_get_field(
              $data_type,
              $stock,
              'femaleParentStockId'
            );
            $female_parent = chado_generate_var(
              'stock',
              array('stock_id' => $female_parent_stock_id)
            );
            if ($female_parent) {
              $pedigree .= $female_parent->name;
            }
            $pedigree .= '/';
            $male_parent_stock_id = brapi_get_field(
              $data_type,
              $stock,
              'maleParentStockId');
            $male_parent = chado_generate_var(
              'stock',
              array('stock_id' => $male_parent_stock_id)
            );
            if ($male_parent) {
              $pedigree .= $male_parent->name;
            }
            return $pedigree;
          },
          'seedSource' => array(
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['SeedSource'],
            ),
            'field_type' => 'string',
          ),
          'synonyms' => array(
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['synonyms'],
            ),
            'field_type' => 'string',
            'return_array' => TRUE,
            'use_ranking' => TRUE,
          ),
          'commonCropName' => array(
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['commonCropName'],
            ),
            'field_type' => 'string',
            'fallback' => 'commonCropName2',
          ),
          'commonCropName2' => array(
            'table' => 'organism',
            'foreign_key' => 'organism_id',
            'object_key' => 'organism_id',
            'column' => 'common_name',
            'selector' => array(),
            'field_type' => 'string',
          ),
          'instituteCode' => array(
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['instituteCode'],
            ),
            'field_type' => 'string',
            'fallback' => 'instituteCode2',
          ),
          'instituteCode2' => array(
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['instituteCode(fallback)'],
            ),
            'field_type' => 'string',
          ),
          'instituteName' => array(
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['instituteName'],
            ),
            'field_type' => 'string',
            'fallback' => 'instituteName2',
          ),
          'instituteName2' => array(
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['instituteName(fallback)'],
            ),
            'field_type' => 'string',
          ),
          'biologicalStatusOfAccessionCode' => array(
            'table' => 'stock_cvterm',
            'column' => 'cvterm_id',
            'selector' => array(
              'cvterm_id' => $cv_settings['BiologicalStatusOfAccessionCode'],
              'is_not' => FALSE,
            ),
            'field_type' => 'cvterm',
          ),
          'countryOfOriginCode' => array(
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['countryOfOriginCode'],
            ),
            'field_type' => 'string',
            'fallback' => 'countryOfOriginCode2',
          ),
          'countryOfOriginCode2' => array(
            'table' => 'stock_cvterm',
            'column' => 'cvterm_id',
            'selector' => array(
              'cvterm_id' => $cv_settings['countryOfOriginCode(fallback)'],
              'is_not' => FALSE,
            ),
            'field_type' => 'cvterm',
          ),
          'typeOfGermplasmStorageCode' => array(
            'table' => 'stock_cvterm',
            'column' => 'cvterm_id',
            'selector' => array(
              'cvterm_id' => $cv_settings['TypeOfGermplasmStorageCode'],
              'is_not' => FALSE,
            ),
            'field_type' => 'cvterm',
            'return_array' => TRUE,
          ),
          'genus' => '',
          'species' => '',
          'speciesAuthority' => '',
          'subtaxa' => '',
          'subtaxaAuthority' => '',
          'donors' => '',
          'acquisitionDate' => '',
        ),
      ),
      'markerprofiles' => array(),
      'allelematrix'   => array(),
      'traits'         => array(),
      'studies'        => array(),
      'attributes'     => array(),
      'programs'       => array(),
      'studyTypes'     => array(),
      'phenotypes'     => array(),
      'variables'      => array(),
      'maps'           => array(),
      'observation'    => array(),
      'locations'      => array(),
    );
  }
  return $brapi_data_mapping;
}
