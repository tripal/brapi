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
define('BRAPI_AGGREGATION_OPTIONS', 'brapi_aggregation_options');
define('BRAPI_USE_PERMISSION', 'use breeding api');
define('BRAPI_UPDATE_PERMISSION', 'update through breeding api');
define('BRAPI_ADMIN_PERMISSION', 'administer breeding api');
define('BRAPI_MULTICROP_PASSPORT_ONTOLOGY_CV', 'multicrop passport ontology');
define('BRAPI_DEFAULT_PAGE_SIZE', 30);
define('BRAPI_ESCAPE_CHARACTER', "\\");
define('BRAPI_XSD_DATE_FORMAT', "Y-m-d");
define('BRAPI_MENU_CALL_MAPPING', "brapi_menu_call_mapping");

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
 * Returns BrAPI/MCPD controlled vocabulary mapping.
 *
 * Returns the association between BrAPI MCPD terms and real multi-crop passport
 * descriptor controlled vocabulary terms.
 *
 * @return array
 *   keys are BrAPI MCPD terms and values are arrays with the following
 *   structure:
 *   - name: corresponding MCPD term name or MCPD term names;
 *   - related: either the parent MCPD term name or an array with the fields
 *     'name' for MCPD parent term name, 'cv' for the CV the related terms
 *     belong to and 'relationship' (string) for the type of relationship;
 *   - fallback: array of terms that could suggested as replacement.
 *
 * @see brapi_get_mcpd_settings()
 *
 * @ingroup brapi_api
 */
function brapi_get_mcpd_mapping() {
  static $mcpd_mapping;
  if (!$mcpd_mapping) {
    $mcpd_mapping = array(
      'accessionNumber' => array('name' => 'accession number'),
      'acquisitionDate' => array('name' => 'acquisition date'),
      'BiologicalStatusOfAccessionCode' => array(
        'related' => 'BiologicalStatusOfAccessionCode',
      ),
      'commonCropName' => array('name' => 'common crop name'),
      'countryOfOriginCode' => array(
        'related' => array(
          'name' => 'countryOfOriginCode',
          'cv' => BRAPI_MULTICROP_PASSPORT_ONTOLOGY_CV,
          'relationship' => 'is_a',
        ),
      ),
      'donor' => array(
        'name' => 'donor institute name',
        'fallback' => array('donor institute code'),
      ),
      'genus' => array('name' => 'genus'),
      'germplasmName' => array('name' => 'accession name'),
      'instituteCode' => array(
        'name' => 'institute code',
        'fallback' => array('breeding institute code', 'InstituteCode'),
      ),
      'instituteName' => array('name' => 'Breeding institute name'),
      'species' => array('name' => 'species'),
      'speciesAuthority' => array('name' => 'species authority'),
      'subtaxa' => array('name' => 'subtaxa'),
      'subtaxaAuthority' => array('name' => 'subtaxa authority'),
      'synonyms' => array(
        'name' => 'other identification numbers associated with the accession',
      ),
      'TypeOfGermplasmStorageCode' => array(
        'related' => 'TypeOfGermplasmStorageCode'
      ),
    );
  }
  return $mcpd_mapping;
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
 *     supported by the call;
 *   - 'aggregatable' (bool): if true, the call can be aggregated with other
 *     BrAPI sites providing the same call.
 *
 * @ingroup brapi_const
 */
function brapi_get_calls() {
  static $brapi_calls;
  if (!isset($brapi_calls)) {
    $brapi_calls = array(
      ''               => array(
        'title'          => 'About',
        'description'    => 'About Breeding API Drupal implementation',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET' => 'access content',
        ),
        'page callback'  => 'brapi_v1_about_json',
        'aggregatable' => FALSE,
      ),
      'calls'      => array(
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
        'aggregatable' => FALSE,
      ),
      'germplasm/{germplasmDbId}' => array(
        'title'          => 'Germplasm Details',
        'description'    => 'Germplasm Details by germplasmDbId',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'    => BRAPI_USE_PERMISSION,
          'POST'   => BRAPI_UPDATE_PERMISSION,
          'PUT'    => BRAPI_UPDATE_PERMISSION,
          'DELETE' => BRAPI_UPDATE_PERMISSION,
        ),
        'page callback'  => 'brapi_v1_germplasm_details_json',
        'arguments'      => array(
          3 => array(
            'name' => 'germplasmDbId',
            'type' => 'string',
            'description' => 'correspond to a Chado stock.uniquename value',
            'required' => FALSE,
          ),
        ),
        'features'       => array('MCPD' => 'yes', 'MCPD-version' => 'V.' . BRAPI_MCPD_VERSION),
        'aggregatable' => TRUE,
      ),
      'germplasm/{germplasmDbId}/markerprofiles' => array(
        'title'          => 'Markerprofiles',
        'description'    => 'Markerprofiles by germplasmDbId',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'    => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => 'brapi_v1_external_call_json',
        'arguments'      => array(
          3 => array(
            'name' => 'germplasmDbId',
            'type' => 'string',
            'description' => 'correspond to a Chado stock.uniquename value',
            'required' => FALSE,
          ),
        ),
        'aggregatable' => TRUE,
      ),
      'germplasm/{germplasmDbId}/pedigree' => array(
        'title'          => 'Germplasm Pedigree',
        'description'    => 'Pedigree by germplasmDbId',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'    => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => 'brapi_v1_germplasm_pedigree_json',
        'arguments'      => array(
          3 => array(
            'name' => 'germplasmDbId',
            'type' => 'string',
            'description' => 'correspond to a Chado stock.uniquename value',
            'required' => FALSE,
          ),
        ),
        'aggregatable' => TRUE,
      ),
      'germplasm-search' => array(
        'title'          => 'Germplasm Search',
        'description'    => 'Germplasm search call',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => 'brapi_v1_germplasm_search_json',
        'features'       => array('MCPD' => 'yes', 'MCPD-version' => 'V.' . BRAPI_MCPD_VERSION),
        'aggregatable' => TRUE,
      ),
      'token'      => array(
        'title'          => 'Authentication',
        'description'    => 'User authentication in OAuth2 style',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'    => BRAPI_USE_PERMISSION,
          'POST'   => BRAPI_USE_PERMISSION,
          'DELETE' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => 'brapi_v1_authentication_json',
        'aggregatable' => FALSE,
      ),
      // Externalized calls.
      'allelematrix-search'      => array(
        'title'          => 'Allele matrix search',
        'description'    => 'Allele matrix search',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => 'brapi_v1_external_call_json',
        'aggregatable' => TRUE,
      ),
      'crops'      => array(
        'title'          => 'Crops',
        'description'    => 'Crops',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => 'brapi_v1_external_call_json',
        'aggregatable' => TRUE,
      ),
      'locations'      => array(
        'title'          => 'Locations',
        'description'    => 'Locations',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => 'brapi_v1_external_call_json',
        'aggregatable' => TRUE,
      ),
      'maps'      => array(
        'title'          => 'Maps',
        'description'    => 'Maps',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => 'brapi_v1_external_call_json',
        'aggregatable' => TRUE,
      ),
      'markerprofiles'      => array(
        'title'          => 'Marker profiles',
        'description'    => 'Marker profiles',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => 'brapi_v1_external_call_json',
        'aggregatable' => TRUE,
      ),
      'markers'      => array(
        'title'          => 'Markers',
        'description'    => 'Markers',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => 'brapi_v1_external_call_json',
        'aggregatable' => TRUE,
      ),
      'observationLevels' => array(
        'title'          => 'Observation levels',
        'description'    => 'Observation levels',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => 'brapi_v1_external_call_json',
        'aggregatable' => TRUE,
      ),
      'ontologies' => array(
        'title'          => 'Ontologies',
        'description'    => 'Ontologies',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => 'brapi_v1_external_call_json',
        'aggregatable' => TRUE,
      ),
      'phenotypes-search'      => array(
        'title'          => 'Phenotypes search',
        'description'    => 'Phenotypes search',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => 'brapi_v1_external_call_json',
        'aggregatable' => TRUE,
      ),
      'programs'      => array(
        'title'          => 'Programs',
        'description'    => 'Programs',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => 'brapi_v1_external_call_json',
        'aggregatable' => TRUE,
      ),
      'samples'      => array(
        'title'          => 'Samples',
        'description'    => 'Samples',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => 'brapi_v1_external_call_json',
        'aggregatable' => TRUE,
      ),
      'seasons'      => array(
        'title'          => 'Seasons',
        'description'    => 'Seasons',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => 'brapi_v1_external_call_json',
        'aggregatable' => TRUE,
      ),
      'studies'      => array(
        'title'          => 'Studies',
        'description'    => 'Studies',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => 'brapi_v1_external_call_json',
        'aggregatable' => TRUE,
      ),
      'studies-search'      => array(
        'title'          => 'Studies search',
        'description'    => 'Studies search',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => 'brapi_v1_external_call_json',
        'aggregatable' => TRUE,
      ),
      'studyTypes'      => array(
        'title'          => 'Study types',
        'description'    => 'Study types',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => 'brapi_v1_external_call_json',
        'aggregatable' => TRUE,
      ),
      'traits'      => array(
        'title'          => 'Traits',
        'description'    => 'Traits',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => 'brapi_v1_external_call_json',
        'aggregatable' => TRUE,
      ),
      'trials'      => array(
        'title'          => 'Trials',
        'description'    => 'Trials',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => 'brapi_v1_external_call_json',
        'aggregatable' => TRUE,
      ),
      'variables'      => array(
        'title'          => 'Variables',
        'description'    => 'Variables',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => 'brapi_v1_external_call_json',
        'aggregatable' => TRUE,
      ),
      'variables-search' => array(
        'title'          => 'Variables search',
        'description'    => 'Variables search',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => 'brapi_v1_external_call_json',
        'aggregatable' => TRUE,
      ),
      'variables/datatypes' => array(
        'title'          => 'Variables datatypes',
        'description'    => 'Variables datatypes',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => 'brapi_v1_external_call_json',
        'aggregatable' => TRUE,
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
 *      - field_type: one of 'string', 'int', 'bool', 'date' and 'cvterm'.
 *        'date' can be litteral or timestamps while the later are turned into
 *        'YYYY/MM/DD' format; 'cvterm' are cvterm_id turned into cvterm.name;
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
 * @see brapi_get_cv_settings()
 * @see brapi_get_mcpd_settings()
 *
 * @ingroup brapi_const
 */
function brapi_get_data_mapping() {
  static $brapi_data_mapping;

  if (!isset($brapi_data_mapping)) {
    $cv_settings = brapi_get_cv_settings();
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
          'parent1DbId' => function ($data_type, $stock, $field_name, $op) {
            $parent_stock_id = brapi_get_field(
              $data_type,
              $stock,
              'femaleParentStockId'
            );
            $parent = chado_generate_var(
              'stock',
              array('stock_id' => $parent_stock_id)
            );
            return ($parent ? $parent->uniquename : '');
          },
          'maleParentStockId' => array(
            'table' => 'stock_relationship',
            'column' => 'subject_id',
            'foreign_key' => 'object_id',
            'selector' => array(
              'type_id' => $cv_settings['MaleParentOf'],
            ),
            'field_type' => 'int',
          ),
          'parent2DbId' => function ($data_type, $stock, $field_name, $op) {
            $parent_stock_id = brapi_get_field(
              $data_type,
              $stock,
              'maleParentStockId'
            );
            $parent = chado_generate_var(
              'stock',
              array('stock_id' => $parent_stock_id)
            );
            return ($parent ? $parent->uniquename : '');
          },
          'pedigree' => function ($data_type, $stock, $field_name, $op) {
            $pedigree = '';
            $female_parent = brapi_get_field(
              $data_type,
              $stock,
              'parent1DbId'
            );
            $male_parent = brapi_get_field(
              $data_type,
              $stock,
              'parent2DbId'
            );
            if ($female_parent || $male_parent) {
              $pedigree = "$female_parent / $male_parent";
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
          'commonCropNameProp' => array(
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['commonCropName'],
            ),
            'field_type' => 'string',
          ),
          'commonCropNameOrganism' => array(
            'table' => 'organism',
            'foreign_key' => 'organism_id',
            'object_key' => 'organism_id',
            'column' => 'common_name',
            'selector' => array(),
            'field_type' => 'string',
          ),
          // From MCPD cvterm 'institute code'.
          'instituteCodeProp' => array(
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['instituteCode'],
            ),
            'field_type' => 'string',
          ),
          // if not specified through MCPD, uses stockcollection.uniquename
          'instituteCodeStockCollection' => function ($data_type, $stock, $field_name, $op) {
            $stock = chado_expand_var($stock, 'table', 'stockcollection_stock');
            if (isset($stock->stockcollection_stock)
                && isset($stock->stockcollection_stock->stockcollection_id)) {
              return $stock->stockcollection_stock->stockcollection_id->uniquename;
            }
            else {
              return NULL;
            }
          },
          // Note: the use of MCPD 'institute code' should be priviledged
          // against the use of 'Breeding institute name'. If 'institute code'
          // is set, then this field should be left empty.
          'instituteNameProp' => array(
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['instituteName'],
            ),
            'field_type' => 'string',
          ),
          // if not specified through MCPD, uses stockcollection.name
          'instituteNameStockCollection' => function ($data_type, $stock, $field_name, $op) {
            $stock = chado_expand_var($stock, 'table', 'stockcollection_stock');
            if (isset($stock->stockcollection_stock)
                && isset($stock->stockcollection_stock->stockcollection_id)) {
              return $stock->stockcollection_stock->stockcollection_id->name;
            }
            else {
              return NULL;
            }
          },
          'biologicalStatusOfAccessionCode' => array(
            'table' => 'stock_cvterm',
            'column' => 'cvterm_id',
            'selector' => array(
              'cvterm_id' => $cv_settings['BiologicalStatusOfAccessionCode'],
              'is_not' => FALSE,
            ),
            'field_type' => 'cvterm',
          ),
          'countryOfOriginCodeProp' => array(
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['countryOfOriginCode'],
            ),
            'field_type' => 'string',
          ),
          'countryOfOriginCodeCVTerm' => array(
            'table' => 'stock_cvterm',
            'column' => 'cvterm_id',
            'selector' => array(
              'cvterm_id' => $cv_settings['countryOfOriginCode'],
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
          // Genus from MCPD.
          'genusProp' => array(
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['genus'],
            ),
            'field_type' => 'string',
          ),
          // Genus from chado.organism.
          'genusOrganism' => array(
            'table' => 'organism',
            'foreign_key' => 'organism_id',
            'object_key' => 'organism_id',
            'column' => 'genus',
            'selector' => array(),
            'field_type' => 'string',
          ),
          // Species from MCPD.
          'speciesProp' => array(
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['species'],
            ),
            'field_type' => 'string',
          ),
          // Species from chado.organism.
          'speciesOrganism' => array(
            'table' => 'organism',
            'foreign_key' => 'organism_id',
            'object_key' => 'organism_id',
            'column' => 'species',
            'selector' => array(),
            'field_type' => 'string',
          ),
          'speciesAuthority' => array(
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['speciesAuthority'],
            ),
            'field_type' => 'string',
          ),
          'subtaxa' => array(
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['subtaxa'],
            ),
            'field_type' => 'string',
          ),
          'subtaxaAuthority' => array(
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['subtaxaAuthority'],
            ),
            'field_type' => 'string',
          ),
          'donors' => array(
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['donor'],
            ),
            'field_type' => 'string',
            'return_array' => TRUE,
          ),
          'acquisitionDate' => array(
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['acquisitionDate'],
            ),
            'field_type' => 'date',
          ),
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

    // Alter default settings.
    $storage_options = variable_get('brapi_storage_options', array());

    // Germplasm fields...
    $germplasm_fields = &$brapi_data_mapping['germplasm']['fields'];

    // Set "common crop name" settings, default to prop.
    $germplasm_fields['commonCropName'] =
      &$germplasm_fields['commonCropNameProp'];
    $use_specified_settings = array_key_exists(
      $storage_options['common_crop_name'],
      $germplasm_fields
    );
    if ($use_specified_settings) {
      $germplasm_fields['commonCropName'] =
        &$germplasm_fields[$storage_options['common_crop_name']];
    }

    // Set "genus" settings, default to prop.
    $germplasm_fields['genus'] =
      &$germplasm_fields['genusProp'];
    $use_specified_settings = array_key_exists(
      $storage_options['genus'],
      $germplasm_fields
    );
    if ($use_specified_settings) {
      $germplasm_fields['genus'] =
        &$germplasm_fields[$storage_options['genus']];
    }

    // Set "species" settings, default to prop.
    $germplasm_fields['species'] =
      &$germplasm_fields['speciesProp'];
    $use_specified_settings = array_key_exists(
      $storage_options['species'],
      $germplasm_fields
    );
    if ($use_specified_settings) {
      $germplasm_fields['species'] =
        &$germplasm_fields[$storage_options['species']];
    }

    // Set "country of origin" settings, default to prop.
    $germplasm_fields['countryOfOriginCode'] =
      &$germplasm_fields['countryOfOriginCodeProp'];
    $use_specified_settings = array_key_exists(
      $storage_options['country_of_origin_code'],
      $germplasm_fields
    );
    if ($use_specified_settings) {
      $germplasm_fields['countryOfOriginCode'] =
        &$germplasm_fields[$storage_options['country_of_origin_code']];
    }

    // Set "institute code" settings, default to prop.
    $germplasm_fields['instituteCode'] =
      &$germplasm_fields['instituteCodeProp'];
    $use_specified_settings = array_key_exists(
      $storage_options['institute_code'],
      $germplasm_fields
    );
    if ($use_specified_settings) {
      $germplasm_fields['instituteCode'] =
        &$germplasm_fields[$storage_options['institute_code']];
    }

    // Set "institute name" settings, default to prop.
    $germplasm_fields['instituteName'] =
      &$germplasm_fields['instituteNameProp'];
    $use_specified_settings = array_key_exists(
      $storage_options['institute_name'],
      $germplasm_fields
    );
    if ($use_specified_settings) {
      $germplasm_fields['instituteName'] =
        &$germplasm_fields[$storage_options['institute_name']];
    }

    // Allow other modules to alter settings.
    drupal_alter('brapi_data_mapping', $brapi_data_mapping);
  }

  return $brapi_data_mapping;
}
