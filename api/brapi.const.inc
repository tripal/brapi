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
define('BRAPI_ADMIN_PERMISSION', 'administer breeding api');
define('BRAPI_AGGREGATION_OPTIONS', 'brapi_aggregation_options');
define('BRAPI_CUSTOM_DATE_FORMAT', 'brapi_custom_date_format');
define('BRAPI_CV', 'breeding api');
define('BRAPI_CV_SETTINGS', 'brapi_cv_settings');
define('BRAPI_DATE_FORMAT', 'brapi_date_format');
define('BRAPI_DB', 'breeding api');
define('BRAPI_DEFAULT_PAGE_SIZE', 30);
define('BRAPI_ESCAPE_CHARACTER', "\\");
define('BRAPI_EXAMPLE_SETTINGS', 'brapi_example_settings');
define('BRAPI_GERMPLASM_ATTR_CATEGORIES', 'brapi_germplasm_attr_categories');
define('BRAPI_GERMPLASM_ATTRIBUTES', 'brapi_germplasm_attributes');
define('BRAPI_MCPD_VERSION', '2.1');
define('BRAPI_MENU_CALL_MAPPING', "brapi_menu_call_mapping");
define('BRAPI_MULTICROP_PASSPORT_ONTOLOGY_CV', 'multicrop passport ontology');
define('BRAPI_SERVICE_PREFIX_V1', 'brapi/v1');
define('BRAPI_SERVICE_VERSION', '1.2');
define('BRAPI_STORAGE_OPTIONS', 'brapi_storage_options');
define('BRAPI_UPDATE_PERMISSION', 'update through breeding api');
define('BRAPI_USE_PERMISSION', 'use breeding api');
define('BRAPI_VERSION_OPTIONS', 'brapi_version_options');
define('BRAPI_XSD_DATE_FORMAT', "Y-m-d");

/**
 * Returns BrAPI versions covered by calls defined in this file.
 *
 * @return array
 *   An array of version machine name => version name.
 */
function brapi_get_versions() {
  static $api_versions;

  if (!isset($api_versions)) {
    foreach (brapi_get_calls() as $call_name => $call_info) {
      foreach ($call_info['api versions'] as $version) {
        $version_mn = preg_replace('/\W+/', '_', $version);
        $api_versions[$version_mn] = $version;
      }
    }
  }
  return $api_versions;
}

/**
 * Tells if a given URL query parameter is not a field filter.
 *
 * @param string $parameter
 *   Parameter name.
 *
 * @return bool
 *   True if the given parameter name is a query parameter and not a filter or
 *   something else.
 */
function brapi_is_query_parameter($parameter) {
  $parameters = array(
    'debug' => TRUE,
    'matchMethod' => TRUE,
    'page' => TRUE,
    'pageSize' => TRUE,
    'pretty' => TRUE,
  );
  return isset($parameters[$parameter]);
}

/**
 * Returns BrAPI CV settings.
 *
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
    'CountryCode'      => 'Geo-location country code (nd_geolocationprop.type_id).',
    'CountryName'      => 'Geo-location country name (nd_geolocationprop.type_id).',
    'DonorGermplasmPUI' => 'Donor germplasm persistent unique identifier.',
    'FemaleParentOf'   => 'Female parent relationship (subject stock is the female parent of the object stock)',
    'Germplasm'        => 'Germplasm type (default stock.type_id)',
    'GermplasmSynonym' => 'Germplasm name synonym (germplasm stockprop.type_id)',
    'InstituteAddress' => 'Address of the institute(used by nd_geolocationprop.type_id).',
    'InstituteName'    => 'Name of the institute(used by nd_geolocationprop.type_id).',
    'LeadPerson'       => 'Type id (contact.type_id) of a lead person of a program (project).',
    'LocationAbbreviation' => 'Geo-location name abbreviation (nd_geolocationprop.type_id).',
    'LocationType'     => 'Geo-location location type type_id (nd_geolocationprop.type_id).',
    'MaleParentOf'     => 'Male parent relationship (subject stock is the male parent of the object stock)',
    'MarkerSynonym'    => 'Marker name synonym (marker featureprop.type_id).',
    'MarkerType'       => 'Marker type names (marker feature.type_id, default to SO term "genetic_marker").',
    'Notes'            => 'Note type (stockprop.type_id).',
    'PlateDbId'        => 'Plate property type (nd_experimentprop.type_id).',
    'PlateIndex'       => 'Plate index property type (nd_experimentprop.type_id).',
    'PlantDbId'        => 'Plant identifier property type (stockprop.type_id).',
    'PlotDbId'         => 'Plot property type (nd_experimentprop.type_id).',
    'Sample'           => 'Sample type (stock.type_id for samples)',
    'SampleOf'         => 'Sample relationship (subject stock is the sample of the object stock)',
    'SampleTimestamp'  => 'Sample timestamp (nd_experimentprop.type_id)',
    'SeedSource'       => 'Name or identifier of the seed provider',
    'Study'            => 'Study property type (nd_experimentprop.type_id).',
    'Trait'            => 'An organism trait (cvterm.name)',
    'TypeOfTissue'     => 'Type of tissue (stock_cvterm.cvterm_id).',
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
      'donor institute' => array(
        'name' => 'donor institute name',
        'fallback' => array('donor institute code'),
      ),
      'donor accession number' => array('name' => 'donor accession number'),
      'genus' => array('name' => 'genus'),
      'germplasmName' => array('name' => 'accession name'),
      'instituteCode' => array(
        'name' => 'institute code',
        'fallback' => array('breeding institute code', 'InstituteCode'),
      ),
      'instituteName' => array('name' => 'Breeding institute name'),
      'persistent unique identifier' => array('name' => 'persistent unique identifier'),
      'species' => array('name' => 'species'),
      'speciesAuthority' => array('name' => 'species authority'),
      'subtaxa' => array('name' => 'subtaxa'),
      'subtaxaAuthority' => array('name' => 'subtaxa authority'),
      'synonyms' => array(
        'name' => 'other identification numbers associated with the accession',
      ),
      'TypeOfGermplasmStorageCode' => array(
        'related' => 'TypeOfGermplasmStorageCode',
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
 *   - 'api versions' (array): list of BrAPI version having the call;
 *   - 'active version': label of the active implementation version of the call.
 *     must be one of the 'callback versions' labels (see below);
 *   - 'callback versions': associative array of callback version labels with
 *     their associated callback function name available;
 *   - 'arguments' (array): an array where indices are the call URL argument
 *     position and values are arrays describing the argument:
 *     - 'name' (string): name of the argument;
 *     - 'type' (string): type of the argument;
 *     - 'description' (string): description of the argument;
 *     - 'required' (bool): true if the argument is required;
 *   - 'parameters' (array): hash of parameter names (array of strings) grouped
 *       by API version (as keys);
 *   - 'filters' (array): hash of API version, each containing a hash of data
 *       type names each containing a list of field names (array of string)
 *       which can be used for filtering;
 *   - 'features' (array): an array of feature name => value/description
 *     supported by the call;
 *   - 'aggregable' (bool): if true, the call can be aggregated with other
 *     BrAPI sites providing the same call.
 *
 * @ingroup brapi_const
 */
function brapi_get_calls() {
  static $brapi_calls;
  if (!isset($brapi_calls)) {
    // Get storage options for *DbId to update descriptions.
    $storage_options = variable_get(BRAPI_STORAGE_OPTIONS, array());
    $dbid_column = '.uniquename';
    if (isset($storage_options['dbid'])
        && ($storage_options['dbid'] == 'chado_id')) {
      $dbid_column = '_id';
    }

    $brapi_calls = array(
      /*********
       * About *
       *********/
      '' => array(
        'title' => 'About',
        'description' => 'About Breeding API Drupal implementation',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => 'access content',
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_root_10_json',
          '1.1' => 'brapi_v1_root_10_json',
          '1.2' => 'brapi_v1_root_10_json',
          '1.3' => 'brapi_v1_root_10_json',
        ),
        'aggregable' => FALSE,
      ),

      /******************
       * Authentication *
       ******************/
      'token' => array(
        'title' => 'Authentication',
        'description' => 'User authentication in OAuth2 style',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
          'DELETE' => BRAPI_UPDATE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_authentication_json',
          '1.1' => 'brapi_v1_authentication_json',
        ),
        'aggregable' => FALSE,
      ),

      /*********
       * Calls *
       *********/
      'calls' => array(
        'title' => 'Call Search',
        'description' => 'List supported Breeding API calls',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_call_10_json',
          '1.1' => 'brapi_v1_call_10_json',
          '1.2' => 'brapi_v1_call_12_json',
          '1.3' => 'brapi_v1_call_13_json',
        ),
        'arguments' => array(
          3 => array(
            'name' => 'callName',
            'type' => 'string',
            'description' => 'a call name (machine name)',
            'required' => FALSE,
          ),
        ),
        'aggregable' => FALSE,
      ),

      /*********
       * Crops *
       *********/
      // Deprecated as of v1.2; Use /commonCropNames instead.
      'crops' => array(
        'title' => 'Crops',
        'description' => 'List supported crops',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_crops_10_json',
          '1.1' => 'brapi_v1_crops_10_json',
          '1.2' => 'brapi_v1_crops_10_json',
          '1.3' => 'brapi_v1_crops_10_json',
        ),
        'aggregable' => TRUE,
      ),

      // Replaces /crops as of v1.2.
      'commonCropNames' => array(
        'title' => 'Common Crop Names',
        'description' => 'List the common crop names for the crops available in the database server.',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.2' => 'brapi_v1_crops_10_json',
          '1.3' => 'brapi_v1_crops_10_json',
        ),
        'aggregable' => TRUE,
      ),

      /****************
       * Genomic Maps *
       ****************/
      'maps' => array(
        'title' => 'Genome Map',
        'description' => 'Get list of maps',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_external_call_json',
          '1.1' => 'brapi_v1_external_call_json',
          '1.2' => 'brapi_v1_external_call_json',
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'maps/{mapDbId}' => array(
        'title' => 'Genome Map Details',
        'description' => 'Get map details',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_external_call_json',
          '1.1' => 'brapi_v1_external_call_json',
          '1.2' => 'brapi_v1_external_call_json',
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'maps/{mapDbId}/positions' => array(
        'title' => 'Genome map data',
        'description' => 'Get map data',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_external_call_json',
          '1.1' => 'brapi_v1_external_call_json',
          '1.2' => 'brapi_v1_external_call_json',
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'maps/{mapDbId}/positions/{linkageGroupName}' => array(
        'title' => 'Genome Map Data by range on linkageGroup',
        'description' => 'Get map data by range on linkageGroup',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_external_call_json',
          '1.1' => 'brapi_v1_external_call_json',
          '1.2' => 'brapi_v1_external_call_json',
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),

      /**************************
       * Germplasm & Attributes *
       **************************/
      'germplasm' => array(
        'title' => 'Germplasm',
        'description' => 'Germplasm list',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_germplasm_10_json',
          '1.2' => 'brapi_v1_germplasm_10_json',
          '1.3' => 'brapi_v1_germplasm_10_json',
        ),
        'features' => array('MCPD' => 'yes', 'MCPD-version' => 'V.' . BRAPI_MCPD_VERSION),
        'aggregable' => TRUE,
      ),
      // Drupal implementation note: Use of 'POST', 'PUT' and 'DELETE' are
      // unspecified in BrAPI (in v1.*).
      'germplasm/{germplasmDbId}' => array(
        'title' => 'Germplasm Details',
        'description' => 'Germplasm details by germplasmDbId',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_UPDATE_PERMISSION,
          'PUT' => BRAPI_UPDATE_PERMISSION,
          'DELETE' => BRAPI_UPDATE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_germplasm_10_json',
          '1.1' => 'brapi_v1_germplasm_10_json',
          '1.2' => 'brapi_v1_germplasm_10_json',
          '1.3' => 'brapi_v1_germplasm_10_json',
        ),
        'arguments' => array(
          3 => array(
            'name' => 'germplasmDbId',
            'type' => 'string',
            'description' => "correspond to a Chado stock$dbid_column value",
            'required' => FALSE,
          ),
        ),
        'features' => array('MCPD' => 'yes', 'MCPD-version' => 'V.' . BRAPI_MCPD_VERSION),
        'aggregable' => TRUE,
      ),
      'search/germplasm' => array(
        'title' => 'Search Germplasm',
        'description' => 'Search for a set of germplasm based on some criteria, returning a search results id',
        'datatypes' => array('json'),
        'methods' => array(
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'search/germplasm/{searchResultsDbId}' => array(
        'title' => 'Get Germplasm Search Results',
        'description' => 'Get results of search/germplasm call by search id',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'attributes' => array(
        'title' => 'Germplasm Attribute List',
        'description' => 'List attributes available',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_attributes_11_json',
          '1.1' => 'brapi_v1_attributes_11_json',
          '1.2' => 'brapi_v1_attributes_11_json',
          '1.3' => 'brapi_v1_attributes_11_json',
        ),
        'aggregable' => TRUE,
      ),
      'attributes/categories' => array(
        'title' => 'Germplasm Attribute Category List',
        'description' => 'List attribute categories available',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_categories_11_json',
          '1.1' => 'brapi_v1_categories_11_json',
          '1.2' => 'brapi_v1_categories_11_json',
          '1.3' => 'brapi_v1_categories_11_json',
        ),
        'aggregable' => TRUE,
      ),
      // This call moved under the 'Germplasm' group in BrAPI v1.3.
      'germplasm/{germplasmDbId}/attributes' => array(
        'title' => 'Germplasm Attributes',
        'description' => 'Germplasm attribute values by germplasmDbId',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_germplasm_germplasmdbid_attributes_10_json',
          '1.1' => 'brapi_v1_germplasm_germplasmdbid_attributes_10_json',
          '1.2' => 'brapi_v1_germplasm_germplasmdbid_attributes_10_json',
          '1.3' => 'brapi_v1_germplasm_germplasmdbid_attributes_10_json',
        ),
        'arguments' => array(
          3 => array(
            'name' => 'germplasmDbId',
            'type' => 'string',
            'description' => "correspond to a Chado stock$dbid_column value",
            'required' => TRUE,
          ),
        ),
        'aggregable' => TRUE,
      ),
      'germplasm/breedingmethods' => array(
        'title' => 'Germplasm Breeding Methods',
        'description' => 'Get the list of germplasm breeding methods available in a system.',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.2' => 'brapi_v1_external_call_json',
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'germplasm/breedingmethods/{breedingMethodDbId}' => array(
        'title' => 'Germplasm Breeding Method',
        'description' => 'Get specific germplasm breeding method by identifier',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.2' => 'brapi_v1_external_call_json',
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'germplasm/{germplasmDbId}/mcpd' => array(
        'title' => 'Germplasm by MCPD',
        'description' => 'Germplasm by Multi-Crop Passport Descriptors',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'germplasm/{germplasmDbId}/markerprofiles' => array(
        'title' => 'Germplasm Markerprofiles',
        'description' => 'Markerprofiles by germplasmDbId',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_external_call_json',
          '1.1' => 'brapi_v1_external_call_json',
          '1.2' => 'brapi_v1_external_call_json',
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'arguments' => array(
          3 => array(
            'name' => 'germplasmDbId',
            'type' => 'string',
            'description' => "correspond to a Chado stock$dbid_column value",
            'required' => TRUE,
          ),
        ),
        'aggregable' => TRUE,
      ),
      'germplasm/{germplasmDbId}/pedigree' => array(
        'title' => 'Germplasm Pedigree',
        'description' => 'Pedigree by germplasmDbId',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_germplasm_germplasmdbid_pedigree_10_json',
          '1.1' => 'brapi_v1_germplasm_germplasmdbid_pedigree_11_json',
          '1.2' => 'brapi_v1_germplasm_germplasmdbid_pedigree_11_json',
          '1.3' => 'brapi_v1_germplasm_germplasmdbid_pedigree_11_json',
        ),
        'arguments' => array(
          3 => array(
            'name' => 'germplasmDbId',
            'type' => 'string',
            'description' => "correspond to a Chado stock$dbid_column value",
            'required' => TRUE,
          ),
        ),
        'aggregable' => TRUE,
      ),
      'germplasm/{germplasmDbId}/progeny' => array(
        'title' => 'Germplasm by Progeny',
        'description' => 'Get the germplasmDbIds for all the Progeny of a particular germplasm',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.2' => 'brapi_v1_external_call_json',
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      // Deprecated in BrAPI v1.3.
      'germplasm-search' => array(
        'title' => 'Germplasm Search',
        'description' => 'Germplasm search',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_germplasm_search_10_json',
          '1.1' => 'brapi_v1_germplasm_search_10_json',
          '1.2' => 'brapi_v1_germplasm_search_10_json',
          '1.3' => 'brapi_v1_germplasm_search_10_json',
        ),
        'parameters' => array(
          '1.0' => array(
            'matchMethod' => array('exact', 'case_insensitive', 'wildcard'),
          ),
          '1.1' => array(
            'matchMethod' => array('exact', 'case_insensitive', 'wildcard'),
          ),
        ),
        'filters' => array(
          '1.0' => array(
            'germplasm' => array(
              'germplasmPUIs',
              'germplasmDbIds',
              'germplasmSpecies',
              'germplasmGenus',
              'germplasmNames',
              'accessionNumbers',
            ),
          ),
          '1.1' => array(
            'germplasm' => array(
              'germplasmDbId',
              'defaultDisplayName',
              'accessionNumber',
              'germplasmName',
              'germplasmPUI',
              'pedigree',
              'seedSource',
              'synonyms',
              'commonCropName',
              'instituteCode',
              'instituteName',
              'biologicalStatusOfAccessionCode',
              'countryOfOriginCode',
              'typeOfGermplasmStorageCode',
              'genus',
              'species',
              'taxonIds',
              'speciesAuthority',
              'subtaxa',
              'subtaxaAuthority',
              'donors',
              'acquisitionDate',
            ),
          ),
          '1.2' => array(
            'germplasm' => array(
              'germplasmDbId',
              'defaultDisplayName',
              'accessionNumber',
              'germplasmName',
              'germplasmPUI',
              'pedigree',
              'seedSource',
              'synonyms',
              'commonCropName',
              'instituteCode',
              'instituteName',
              'biologicalStatusOfAccessionCode',
              'countryOfOriginCode',
              'typeOfGermplasmStorageCode',
              'genus',
              'species',
              'taxonIds',
              'speciesAuthority',
              'subtaxa',
              'subtaxaAuthority',
              'donors',
              'acquisitionDate',
            ),
          ),
        ),
        'features' => array('MCPD' => 'yes', 'MCPD-version' => 'V.' . BRAPI_MCPD_VERSION),
        'aggregable' => TRUE,
      ),

      /**********
       * Images *
       **********/
      'images' => array(
        'title' => 'Images',
        'description' => 'Get/Create filtered set of image meta data',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'images/{imageDbId}' => array(
        'title' => 'Images',
        'description' => 'Get/Update one image meta data',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          'PUT' => BRAPI_UPDATE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'images/{imageDbId}/imagecontent' => array(
        'title' => 'Images',
        'description' => 'Update an image with the image file content',
        'datatypes' => array('json'),
        'methods' => array(
          'PUT' => BRAPI_UPDATE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'search/images' => array(
        'title' => 'Search Images',
        'description' => 'Search for a set of images based on some criteria, returning a search results id',
        'datatypes' => array('json'),
        'methods' => array(
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'search/images/{searchResultsDbId}' => array(
        'title' => 'Search Images',
        'description' => 'Get results of search/images call by search id',
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),

      /*****************
       * Generic Lists *
       *****************/
      'lists' => array(
        'title' => 'Images',
        'description' => 'Get/Create filtered set of generic lists',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'lists/{listDbId}' => array(
        'title' => 'Images',
        'description' => 'Get/Update one generic lists',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          'PUT' => BRAPI_UPDATE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'lists/{listDbId}/items' => array(
        'title' => 'Images',
        'description' => 'Add new data to a specific generic lists',
        'datatypes' => array('json'),
        'methods' => array(
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),

      /*************
       * Locations *
       *************/
      'locations' => array(
        'title' => 'Locations',
        'description' => 'List Locations',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_locations_11_json',
          '1.1' => 'brapi_v1_locations_11_json',
          '1.2' => 'brapi_v1_locations_11_json',
          '1.3' => 'brapi_v1_locations_11_json',
        ),
        'aggregable' => TRUE,
      ),
      'locations/{locationDbId}' => array(
        'title' => 'Locations Details',
        'description' => 'Locations details by locationDbId',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'arguments' => array(
          3 => array(
            'name' => 'locationDbId',
            'type' => 'string',
            'description' => 'correspond to a Chado nd_geolocation.nd_geolocation_id value',
            'required' => FALSE,
          ),
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_locations_11_json',
          '1.1' => 'brapi_v1_locations_11_json',
          '1.2' => 'brapi_v1_locations_11_json',
          '1.3' => 'brapi_v1_locations_11_json',
        ),
        'aggregable' => TRUE,
      ),

      /*******************
       * Marker Profiles *
       *******************/
      'allelematrices' => array(
        'title' => 'Allele Matrices',
        'description' => 'List allele matrices',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_external_call_json',
          '1.1' => 'brapi_v1_external_call_json',
          '1.2' => 'brapi_v1_external_call_json',
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'allelematrices-search' => array(
        'title' => 'MarkerProfile Allele Matrix',
        'description' => 'Search allele matrices',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_external_call_json',
          '1.1' => 'brapi_v1_external_call_json',
          '1.2' => 'brapi_v1_external_call_json',
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      // Deprecated. Use allelematrices-search instead.
      'allelematrix-search' => array(
        'title' => 'MarkerProfile Allele Matrix',
        'description' => 'Search allele matrices',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'markerprofiles' => array(
        'title' => 'Markerprofile Search',
        'description' => 'List marker profiles',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_external_call_json',
          '1.1' => 'brapi_v1_external_call_json',
          '1.2' => 'brapi_v1_external_call_json',
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'markerprofiles/{markerprofileDbId}' => array(
        'title' => 'Marker profiles',
        'description' => 'Alleles by markerprofileDbId',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_external_call_json',
          '1.1' => 'brapi_v1_external_call_json',
          '1.2' => 'brapi_v1_external_call_json',
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),

      /***********
       * Markers *
       ***********/
      'markers' => array(
        'title' => 'Marker List',
        'description' => 'List markers',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          // Strangely enough, this endpoint seems missing from v1.1 and v1.2.
          '1.0' => 'brapi_v1_markers_11_json',
          '1.3' => 'brapi_v1_markers_11_json',
        ),
        'aggregable' => TRUE,
      ),
      'markers/{markerDbId}' => array(
        'title' => 'Marker Details',
        'description' => 'Markers details by markerDbId',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_markers_11_json',
          '1.1' => 'brapi_v1_markers_11_json',
          '1.2' => 'brapi_v1_markers_11_json',
          '1.3' => 'brapi_v1_markers_11_json',
        ),
        'arguments' => array(
          3 => array(
            'name' => 'markerDbId',
            'type' => 'string',
            'description' => "correspond to a Chado feature$dbid_column value for feature type \"genetic_marker\"",
            'required' => FALSE,
          ),
        ),
        'aggregable' => TRUE,
      ),
      // Unspecified in v1.0 but deprecated, from v1.3.
      'markers-search' => array(
        'title' => 'Markers Search',
        'description' => 'Search markers',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_markers_search_11_json',
          '1.2' => 'brapi_v1_markers_search_11_json',
          '1.3' => 'brapi_v1_markers_search_11_json',
        ),
        'aggregable' => TRUE,
      ),
      'search/markers' => array(
        'title' => 'Search Markers',
        'description' => 'Search for a set of markers based on some criteria, returning a search results id',
        'datatypes' => array('json'),
        'methods' => array(
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'search/markers/{searchResultsDbId}' => array(
        'title' => 'Get Markers Search Results',
        'description' => 'Get results of search/markers call by search id',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),

      /*************************
       * Observation Variables *
       *************************/
      'ontologies' => array(
        'title' => 'Variable ontology list',
        'description' => 'List ontologies',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_external_call_json',
          '1.1' => 'brapi_v1_external_call_json',
          '1.2' => 'brapi_v1_external_call_json',
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'variables' => array(
        'title' => 'Variable List',
        'description' => 'List variables',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_variables_11_json',
          '1.1' => 'brapi_v1_variables_11_json',
          '1.2' => 'brapi_v1_variables_11_json',
          '1.3' => 'brapi_v1_variables_11_json',
        ),
        'aggregable' => TRUE,
      ),
      'variables/{observationVariableDbId}' => array(
        'title' => 'Variable Details',
        'description' => 'Variable details by observationVariableDbId',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_variables_11_json',
          '1.1' => 'brapi_v1_variables_11_json',
          '1.2' => 'brapi_v1_variables_11_json',
          '1.3' => 'brapi_v1_variables_11_json',
        ),
        'arguments' => array(
          3 => array(
            'name' => 'observationVariableDbId',
            'type' => 'string',
            'description' => 'n/a',
            'required' => FALSE,
          ),
        ),
        'aggregable' => TRUE,
      ),
      // Deprecated in v1.3.
      'variables/datatypes' => array(
        'title' => 'Variable Data Types',
        'description' => 'Variable data type list',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_variables_datatypes_11_json',
          '1.1' => 'brapi_v1_variables_datatypes_11_json',
          '1.2' => 'brapi_v1_variables_datatypes_11_json',
          '1.3' => 'brapi_v1_variables_datatypes_11_json',
        ),
        'aggregable' => TRUE,
      ),
      // Deprecated in v1.3; use search/variables endpoint.
      'variables-search' => array(
        'title' => 'Variable Search',
        'description' => 'Search variables',
        'datatypes' => array('json'),
        'methods' => array(
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_variables_search_11_json',
          '1.1' => 'brapi_v1_variables_search_11_json',
          '1.2' => 'brapi_v1_variables_search_11_json',
        ),
        'aggregable' => TRUE,
      ),
      'search/variables' => array(
        'title' => 'Search Germplasm',
        'description' => 'Search for a set of Observation Variables based on some criteria, returning a search results id',
        'datatypes' => array('json'),
        'methods' => array(
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'search/variables/{searchResultsDbId}' => array(
        'title' => 'Get Germplasm Search Results',
        'description' => 'Get results of search/variables call by search id',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),

      /**********************************
       * Observation Variables - Traits *
       **********************************/
      // 'traits' endpoint relocated into 'Observation Variables/Traits' group
      // in v1.3.
      'traits' => array(
        'title' => 'Trait List',
        'description' => 'List all traits',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_traits_11_json',
          '1.1' => 'brapi_v1_traits_11_json',
          '1.2' => 'brapi_v1_traits_11_json',
          '1.3' => 'brapi_v1_traits_11_json',
        ),
        'aggregable' => TRUE,
      ),
      // 'traits' endpoint relocated into 'Observation Variables/Traits' group
      // in v1.3.
      'traits/{traitDbId}' => array(
        'title' => 'Trait Details',
        'description' => 'Trait details by traitDbId',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          // Note: 'PUT' only added in v1.3.
          'PUT' => BRAPI_UPDATE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_traits_11_json',
          '1.1' => 'brapi_v1_traits_11_json',
          '1.2' => 'brapi_v1_traits_11_json',
          '1.3' => 'brapi_v1_traits_11_json',
        ),
        'arguments' => array(
          3 => array(
            'name' => 'traitDbId',
            'type' => 'string',
            'description' => 'n/a',
            'required' => FALSE,
          ),
        ),
        'aggregable' => TRUE,
      ),

      /**********************************
       * Observation Variables - Scales *
       **********************************/
      'scales' => array(
        'title' => 'Scale List',
        'description' => 'List all scales',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_scales_11_json',
        ),
        'aggregable' => TRUE,
      ),
      'scales/{scaleDbId}' => array(
        'title' => 'Scale Details',
        'description' => 'Get/Update Scale details by scaleDbId',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          'PUT' => BRAPI_UPDATE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_scales_13_json',
        ),
        'arguments' => array(
          3 => array(
            'name' => 'scaleDbId',
            'type' => 'string',
            'description' => 'n/a',
            'required' => FALSE,
          ),
        ),
        'aggregable' => TRUE,
      ),

      /***********************************
       * Observation Variables - Methods *
       ***********************************/
      'methods' => array(
        'title' => 'Method List',
        'description' => 'List all Methods',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_methods_13_json',
        ),
        'aggregable' => TRUE,
      ),
      'methods/{methodDbId}' => array(
        'title' => 'Get/Update Method Details',
        'description' => 'Get/Update Method details by methodDbId',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          'PUT' => BRAPI_UPDATE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_methods_13_json',
        ),
        'arguments' => array(
          3 => array(
            'name' => 'methodDbId',
            'type' => 'string',
            'description' => 'n/a',
            'required' => FALSE,
          ),
        ),
        'aggregable' => TRUE,
      ),

      /**********
       * People *
       **********/
      'people' => array(
        'title' => 'Images',
        'description' => 'Get/Create filtered set of people records',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'people/{personDbId}' => array(
        'title' => 'Images',
        'description' => 'Get/Update one person',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          'PUT' => BRAPI_UPDATE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),

      /**************
       * Phenotypes *
       **************/
      'phenotypes' => array(
        'title' => 'Save Phenotypes',
        'description' => 'Save phenotypes for many studies',
        'datatypes' => array('json'),
        'methods' => array(
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      // Deprecated endpoint from v1.3; Use new /search methods below.
      'phenotypes-search' => array(
        'title' => 'Phenotype Search',
        'description' => 'Search phenotypes',
        'datatypes' => array('json'),
        'methods' => array(
          // Note: GET only supported from v1.2.
          'GET' => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      // Deprecated endpoint from v1.3; Use 'search/observationtables' instead.
      'phenotypes-search/csv' => array(
        'title' => 'Phenotype Search (CSV)',
        'description' => 'Search phenotypes returning a CSV formatted file',
        'datatypes' => array('json'),
        'methods' => array(
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.2' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      // Deprecated endpoint from v1.3; Use 'search/observationtables' instead.
      'phenotypes-search/tsv' => array(
        'title' => 'Phenotype Search (TSV)',
        'description' => 'Search phenotypes returning a TSV formatted file',
        'datatypes' => array('json'),
        'methods' => array(
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.2' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      // Deprecated endpoint from v1.3; Use 'search/observationtables' instead.
      'phenotypes-search/table' => array(
        'title' => 'Phenotype Search (Table as JSON)',
        'description' => 'Search phenotypes returning JSON formatted file',
        'datatypes' => array('json'),
        'methods' => array(
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.2' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'observationunits' => array(
        'title' => 'Observation Unit List',
        'description' => 'Get a set of observation units',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'search/observationtables' => array(
        'title' => 'Search Observation Tables ',
        'description' => 'Search for a set of Observation Units based on some criteria, to be retrieved as a formatted table of results of a  preferred format; returns a search results id',
        'datatypes' => array('json'),
        'methods' => array(
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'search/observationtables/{searchResultsDbId}' => array(
        'title' => 'Get Observation Tables Search Results',
        'description' => 'Get results of search/observationtables call by search id',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'search/observationunits' => array(
        'title' => 'Search Observation Units',
        'description' => 'Globally search for a set of Observation Units based on some criteria, to be retrieved as JSON results;eturnsa search results id',
        'datatypes' => array('json'),
        'methods' => array(
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'search/observationunits/{searchResultsDbId}' => array(
        'title' => 'Get Observation Units Search Results',
        'description' => 'Get results of search/observationunits call by search id',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),

      /************
       * Programs *
       ************/
      'programs' => array(
        'title' => 'Program List',
        'description' => 'List programs',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_programs_11_json',
        ),
        'aggregable' => TRUE,
      ),
      'programs/{programDbId}' => array(
        'title' => 'Program Details',
        'description' => 'Program details by programDbId',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_programs_11_json',
        ),
        'arguments' => array(
          3 => array(
            'name' => 'programDbId',
            'type' => 'string',
            'description' => 'correspond to a Chado project.project_id.',
            'required' => FALSE,
          ),
        ),
        'aggregable' => TRUE,
      ),
      // Deprecated as of v1.3; Use search/programs instead.
      'programs-search' => array(
        'title' => 'Program Search',
        'description' => 'Search programs',
        'datatypes' => array('json'),
        'methods' => array(
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_programs_search_11_json',
        ),
        'aggregable' => TRUE,
      ),
      'search/programs' => array(
        'title' => 'Search Breeding Programs',
        'description' => 'Search for a set of breeding programs based on some criteria, returning a search results id',
        'datatypes' => array('json'),
        'methods' => array(
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'search/programs/{searchResultsDbId}' => array(
        'title' => 'Get Breeding Programs Search Results',
        'description' => 'Get results of search/programs call by search id',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),

      /***********
       * Samples *
       **********/
      // Note: BrAPI defines sample creation as a 'PUT' REST operation rather
      // than the standard 'POST' convention.
      'samples' => array(
        'title' => 'Sample List',
        'description' => 'Sample list call',
        'datatypes' => array('json'),
        'methods' => array(
          // Note: GET operation on this endpoint only defined as of v1.3.
          'GET' => BRAPI_USE_PERMISSION,
          'PUT' => BRAPI_UPDATE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_samples_11_json',
          '1.1' => 'brapi_v1_samples_11_json',
          '1.2' => 'brapi_v1_samples_11_json',
          '1.3' => 'brapi_v1_samples_13_json',
        ),
        'aggregable' => TRUE,
      ),
      // Drupal implementation note: sample 'POST', 'PUT' and 'DELETE' are not
      // actually (yet) defined in BrAPI for this endpoint (as of v1.3).
      'samples/{sampleDbId}' => array(
        'title' => 'Sample Details',
        'description' => 'Sample details by sampleDbId',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_UPDATE_PERMISSION,
          'PUT' => BRAPI_UPDATE_PERMISSION,
          'DELETE' => BRAPI_UPDATE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_samples_11_json',
          '1.1' => 'brapi_v1_samples_11_json',
          '1.2' => 'brapi_v1_samples_11_json',
          '1.3' => 'brapi_v1_samples_11_json',
        ),
        'arguments' => array(
          3 => array(
            'name' => 'sampleDbId',
            'type' => 'string',
            'description' => "correspond to a Chado stock$dbid_column value",
            'required' => FALSE,
          ),
        ),
        'aggregable' => TRUE,
      ),
      // Deprecated in v1.3; See GET /samples instead.
      'samples-search' => array(
        'title' => 'Sample Search',
        'description' => 'Search samples',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_samples_search_11_json',
          '1.2' => 'brapi_v1_samples_search_11_json',
          '1.3' => 'brapi_v1_samples_search_11_json',
        ),
        'aggregable' => TRUE,
      ),
      'search/samples' => array(
        'title' => 'Search Samples',
        'description' => 'Search for a set of samples based on some criteria, returning a search results id',
        'datatypes' => array('json'),
        'methods' => array(
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'search/samples/{searchResultsDbId}' => array(
        'title' => 'Get Samples Search Results',
        'description' => 'Get results of search/samples call by search id',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),

      /***********
       * Studies *
       ***********/
      'studies' => array(
        'title' => 'Study List',
        'description' => 'List studies',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'studies/{studyDbId}' => array(
        'title' => 'Study Details',
        'description' => 'Studies details by studyDbId',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_external_call_json',
          '1.1' => 'brapi_v1_external_call_json',
          '1.2' => 'brapi_v1_external_call_json',
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'arguments' => array(
          3 => array(
            'name' => 'studyDbId',
            'type' => 'string',
            'description' => 'n/a',
            'required' => FALSE,
          ),
        ),
        'aggregable' => TRUE,
      ),
      'studies/{studyDbId}/germplasm' => array(
        'title' => 'Study Germplasm Details',
        'description' => 'Study germplasm details by studyDbId',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_external_call_json',
          '1.1' => 'brapi_v1_external_call_json',
          '1.2' => 'brapi_v1_external_call_json',
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'arguments' => array(
          3 => array(
            'name' => 'studyDbId',
            'type' => 'string',
            'description' => 'n/a',
            'required' => TRUE,
          ),
        ),
        'aggregable' => TRUE,
      ),
      // Singular 'layout' version of this call deprecated in v1.3.
      'studies/{studyDbId}/layout' => array(
        'title' => 'Plot Layout Details',
        'description' => 'Retrieve plot layout details',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          // PUT added in v1.2.
          'PUT' => BRAPI_UPDATE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_external_call_json',
          '1.1' => 'brapi_v1_external_call_json',
          '1.2' => 'brapi_v1_external_call_json',
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'arguments' => array(
          3 => array(
            'name' => 'studyDbId',
            'type' => 'string',
            'description' => 'n/a',
            'required' => TRUE,
          ),
        ),
        'aggregable' => TRUE,
      ),
      // Plural 'layouts' endpoint only introduced in v1.3.
      'studies/{studyDbId}/layouts' => array(
        'title' => 'Plot Layout Details',
        'description' => 'Retrieve plot layout details',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          'PUT' => BRAPI_UPDATE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'arguments' => array(
          3 => array(
            'name' => 'studyDbId',
            'type' => 'string',
            'description' => 'n/a',
            'required' => TRUE,
          ),
        ),
        'aggregable' => TRUE,
      ),
      'studies/{studyDbId}/observations' => array(
        'title' => 'Observations Details',
        'description' => 'Get observation units by observation variable by studyDbId',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          // POST call deprecated in v1.1.
          'POST' => BRAPI_USE_PERMISSION,
          'PUT' => BRAPI_UPDATE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_external_call_json',
          '1.1' => 'brapi_v1_external_call_json',
          '1.2' => 'brapi_v1_external_call_json',
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'arguments' => array(
          3 => array(
            'name' => 'studyDbId',
            'type' => 'string',
            'description' => 'n/a',
            'required' => TRUE,
          ),
        ),
        'aggregable' => TRUE,
      ),
      'studies/{studyDbId}/observationunits' => array(
        'title' => 'Observation Units Details',
        'description' => 'Get all observation units by studyDbId',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          // POST deprecated for PUT call in v1.1.
          'POST' => BRAPI_USE_PERMISSION,
          // PUT introduced in v1.1.
          'PUT' => BRAPI_UPDATE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_external_call_json',
          '1.1' => 'brapi_v1_external_call_json',
          '1.2' => 'brapi_v1_external_call_json',
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'arguments' => array(
          3 => array(
            'name' => 'studyDbId',
            'type' => 'string',
            'description' => 'n/a',
            'required' => TRUE,
          ),
        ),
        'aggregable' => TRUE,
      ),
      'studies/{studyDbId}/observationunits/zip' => array(
        'title' => 'Study Observations Zip File  Upload',
        'description' => 'uploading new Observations as a batched Zip File to a system',
        'datatypes' => array('json'),
        'methods' => array(
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
          '1.2' => 'brapi_v1_external_call_json',
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),

      // Camelcase 'observationVariables' endpoint name deprecated after v1.0.
      // Use observationvariables instead.
      'studies/{studyDbId}/observationVariables' => array(
        'title' => 'Study Observation Variables',
        'description' => 'Get observation variables by studyDbId',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_external_call_json',
          '1.1' => 'brapi_v1_external_call_json',
          '1.2' => 'brapi_v1_external_call_json',
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'arguments' => array(
          3 => array(
            'name' => 'studyDbId',
            'type' => 'string',
            'description' => 'n/a',
            'required' => TRUE,
          ),
        ),
        'aggregable' => TRUE,
      ),
      'studies/{studyDbId}/observationvariables' => array(
        'title' => 'Study Observation Variables',
        'description' => 'Get observation variables by studyDbId',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
          '1.2' => 'brapi_v1_external_call_json',
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'arguments' => array(
          3 => array(
            'name' => 'studyDbId',
            'type' => 'string',
            'description' => 'n/a',
            'required' => TRUE,
          ),
        ),
        'aggregable' => TRUE,
      ),
      'studies/{studyDbId}/table' => array(
        'title' => 'Study Observation Units as a Table',
        'description' => 'Retrieve study observation units as table',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_external_call_json',
          '1.1' => 'brapi_v1_external_call_json',
          '1.2' => 'brapi_v1_external_call_json',
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'arguments' => array(
          3 => array(
            'name' => 'studyDbId',
            'type' => 'string',
            'description' => 'n/a',
            'required' => TRUE,
          ),
        ),
        'aggregable' => TRUE,
      ),
      // Deprecated from v1.3.
      'studies-search' => array(
        'title' => 'Study search',
        'description' => 'Search studies',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_external_call_json',
          '1.1' => 'brapi_v1_external_call_json',
          '1.2' => 'brapi_v1_external_call_json',
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'search/studies' => array(
        'title' => 'Search Studies',
        'description' => 'Search for a set of Studies based on some criteria, returning a search results id',
        'datatypes' => array('json'),
        'methods' => array(
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'search/studies/{searchResultsDbId}' => array(
        'title' => 'Get Studies Search Results',
        'description' => 'Get results of search/studies call by search id',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      // CamelCase 'studyTypes' endpoint deprecated in v1.3.
      // Use studytypes call instead.
      'studyTypes' => array(
        'title' => 'Study Type List',
        'description' => 'List study types',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_external_call_json',
          '1.1' => 'brapi_v1_external_call_json',
          '1.2' => 'brapi_v1_external_call_json',
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      // Lower case 'studytypes' endpoint only introduced in v1.3.
      'studytypes' => array(
        'title' => 'Study Type List',
        'description' => 'List study types',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      // CamelCase 'observationLevels' endpoint name deprecated after v1.0.
      // Use observationlevels call instead.
      'observationLevels' => array(
        'title' => 'Observation Level List',
        'description' => 'List observation levels',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_external_call_json',
          '1.1' => 'brapi_v1_external_call_json',
          '1.2' => 'brapi_v1_external_call_json',
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'observationlevels' => array(
        'title' => 'Observation Level List',
        'description' => 'List observation levels',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
          '1.2' => 'brapi_v1_external_call_json',
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'seasons' => array(
        'title' => 'Season List',
        'description' => 'List seasons',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_external_call_json',
          '1.1' => 'brapi_v1_external_call_json',
          '1.2' => 'brapi_v1_external_call_json',
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),

      /**********
       * Trials *
       **********/
      'trials' => array(
        'title' => 'Trial List',
        'description' => 'Retrieve a filtered list of Trials. A Trial is a collection of studies',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_external_call_json',
          '1.1' => 'brapi_v1_external_call_json',
          '1.2' => 'brapi_v1_external_call_json',
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'trials/{trialDbId}' => array(
        'title' => 'Trial Details',
        'description' => 'Get Trial By trialDbId',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.0', '1.1', '1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.0' => 'brapi_v1_external_call_json',
          '1.1' => 'brapi_v1_external_call_json',
          '1.2' => 'brapi_v1_external_call_json',
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'arguments' => array(
          3 => array(
            'name' => 'trialDbId',
            'type' => 'string',
            'description' => 'n/a',
            'required' => FALSE,
          ),
        ),
        'aggregable' => TRUE,
      ),

      /**********************************
       * External Vendor Samples (v1.2) *
       **********************************/
       // Note: Changed drastically in v1.3.
       // See "new" Vendor Samples Group below.
       // Deprecated 'singular' plate endpoints not documented here.
      'vendors/plates/{vendorPlateDbId}' => array(
        'title' => 'Plate Details',
        'description' => 'Get plate details by vendorPlateId',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.2'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.2' => 'brapi_v1_external_call_json',
        ),
        'arguments' => array(
          3 => array(
            'name' => 'vendorPlateDbId',
            'type' => 'string',
            'description' => 'n/a',
            'required' => TRUE,
          ),
        ),
        'aggregable' => TRUE,
      ),

      /*************************
       * Vendor Samples (v1.3) *
       *************************/
      // Small amount of overlap with the above endpoints.
      // This endpoint shared between v1.2 and v1.3 but implementation likely
      // diverges.
      'vendors/specifications' => array(
        'title' => 'Vendor Specifications',
        'description' => 'Get vendor specifications',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.2', '1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.2' => 'brapi_v1_external_call_json',
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),

      /**************************
       * Vendor Samples (v1.3+) *
       **************************/
      'vendors/orders' => array(
        'title' => 'Plate Search',
        'description' => 'Submit an new order/List current available orders to a vendor',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'vendors/orders/{orderId}/plates' => array(
        'title' => 'Retrieve Order Details',
        'description' => 'Retrieve the plate and sample details of an order being processed',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'vendors/orders/{orderId}/results' => array(
        'title' => 'Retrieve Order Results',
        'description' => 'Retrieve the data files generated by the vendors analysis',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'vendors/orders/{orderId}/status' => array(
        'title' => 'Retrieve Order Status',
        'description' => 'Retrieve the current status of an order being processed',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'vendors/plates' => array(
        'title' => 'Plate Search',
        'description' => 'Submit a new set of Sample data',
        'datatypes' => array('json'),
        'methods' => array(
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
      'vendors/plates/{submissionId}' => array(
        'title' => 'Plate Search',
        'description' => 'Get data for a submitted set of plates',
        'datatypes' => array('json'),
        'methods' => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'api versions' => array('1.3'),
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.3' => 'brapi_v1_external_call_json',
        ),
        'aggregable' => TRUE,
      ),
    );

    // Update function active versions according to settings.
    $version_options = variable_get(BRAPI_VERSION_OPTIONS, array());
    foreach ($brapi_calls as $call => &$call_settings) {
      if (isset($version_options[$call])) {
        $call_settings['active version'] = $version_options[$call];
      }
    }

  }

  // Allow other modules to alter call settings.
  drupal_alter('brapi_calls', $brapi_calls);

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
 *   - selector (optional): a Tripal selector as described here as '$values':
 *     http://api.tripal.info/api/tripal/tripal_core%21api%21tripal_core.chado_query.api.inc/group/tripal_chado_query_api/2.x
 *     This selector allows to pre-filter resources (discreminate stock or
 *     CV terms according to the data type for instance).
 *   - 'fields': an array of field configuration (see below);
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
 *      - field_type: one of 'string', 'int', 'float', 'bool', 'date' and
 *        'cvterm'.
 *        'date' can be litteral or timestamps while the later are turned into
 *        'YYYY/MM/DD' format; 'cvterm' are cvterm_id turned into cvterm.name;
 *      - return_array: tells if an array of values should be returned;
 *      - use_ranking: tells if the field table has a rank column to use;
 *      - fallback: another field name (or ordered array of field names) that
 *        can be used to fetch a value if none has been found with current
 *        field;
 *      - alias_for: if set to another field name, current field settings will
 *        be ignored and the other field will be used instead. When using this
 *        value, make sure other fields are not alias for this field and if they
 *        are, they should be updated to be aliases for the field used here as
 *        alias target in order to avoid risks of infinite loops;
 *   - a closure: the closure is run to fetch or store the field value with the
 *     following arguments:
 *      - $data_type: the type of record in the BrAPI terminology (germplasm,
 *        study, ...);
 *      - $object: the corresponding Chado object loaded from database or an
 *        array of objects for the 'filter' operation;
 *      - $field_name: the name of the record field to work on;
 *      - $op: the operation to perform. Can be one of 'create', 'read',
 *         'update', 'delete' and 'filter'. If not provided, 'read' is assumed;
 *      - $value (not provided for 'read' $op and optional for 'delete' $op):
 *        the new field value (or value to delete in case of 'delete' $op) or
 *        the values to use for filtering.
 *     If the function is called with a NULL operation, the function must return
 *     an associative array with the supported operations as keys and TRUE as
 *     values.
 *
 * @see brapi_get_cv_settings()
 * @see brapi_get_mcpd_settings()
 * @see https://www.ncbi.nlm.nih.gov/pmc/articles/PMC4795932/
 *
 * @ingroup brapi_const
 */
function brapi_get_data_mapping() {
  static $brapi_data_mapping;

  if (!isset($brapi_data_mapping)) {
    // Get settings.
    $storage_options = variable_get(BRAPI_STORAGE_OPTIONS, array());

    $cv_settings = brapi_get_cv_settings();
    $organism_info_func = function ($data_type, $stock, $field_name, $op, $values = NULL) {
      $data = '';
      switch ($op) {
        case NULL:
          $data = array('read' => TRUE);
          break;

        case 'read':
          $organism_info = brapi_get_organism_info($stock->organism_id->organism_id);
          $data = $organism_info[$field_name];
          break;

        default:
      }
      return $data;
    };
    $subtaxon_info_func = function ($data_type, $stock, $field_name, $op, $values = NULL) {
      $data = '';
      switch ($op) {
        case NULL:
          $data = array('read' => TRUE);
          break;

        case 'read':
          $organism_info = brapi_get_subtaxon_info($stock->organism_id->organism_id);
          $data = $organism_info[$field_name];
          break;

        default:
      }
      return $data;
    };

    $brapi_data_mapping = array(
      'germplasm'      => array(
        'table' => 'stock',
        'identifier' => 'stock_id',
        'accession' => 'uniquename',
        'selector' => array(
          'type_id' => $cv_settings['Germplasm'],
        ),
        'fields' => [
          // Table fields.
          'defaultDisplayName' => 'name',
          'germplasmDbId' => 'uniquename',

          // Aliases.
          'accessionNumbers' => [
            'alias_for' => 'accessionNumber',
          ],
          'commonCropName' => [
            // Default settings updated later.
            'alias_for' => 'commonCropNameProp',
          ],
          'countryOfOriginCode' => [
            // Default settings updated later.
            'alias_for' => 'countryOfOriginCodeProp',
          ],
          'genus' => [
            // Default settings updated later.
            'alias_for' => 'genusProp',
          ],
          'germplasmDbIds' => [
            'alias_for' => 'germplasmDbId',
          ],
          'germplasmGenus' => [
            'alias_for' => 'genus',
          ],
          'germplasmNames' => [
            'alias_for' => 'germplasmName',
          ],
          'germplasmPUIs' => [
            'alias_for' => 'germplasmPUI',
          ],
          'germplasmSpecies' => [
            'alias_for' => 'species',
          ],
          'instituteCode' => [
            // Default settings updated later.
            'alias_for' => 'instituteCodeProp',
          ],
          'instituteName' => [
            // Default settings updated later.
            'alias_for' => 'instituteNameProp',
          ],
          'species' => [
            // Default settings updated later.
            'alias_for' => 'speciesFunc',
          ],
          'speciesAuthority' => [
            // Default settings updated later.
            'alias_for' => 'speciesAuthorityFunc',
          ],
          'subtaxa' => [
            // Default settings updated later.
            'alias_for' => 'subtaxaFunc',
          ],
          'subtaxaAuthority' => [
            // Default settings updated later.
            'alias_for' => 'subtaxaAuthorityFunc',
          ],
          'taxonIds' => [
            'alias_for' => 'taxonId',
          ],

          // Table joins.
          'accessionNumber' => [
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => [
              'type_id' => $cv_settings['accessionNumber'],
            ],
            'field_type' => 'string',
          ],
          'acquisitionDate' => [
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => [
              'type_id' => $cv_settings['acquisitionDate'],
            ],
            'field_type' => 'date',
          ],
          // Legacy simple cvterm access to biologicalStatusOfAccessionCode.
          'biologicalStatusOfAccessionCodeCVTerm' => [
            'table' => 'stock_cvterm',
            'column' => 'cvterm_id',
            'selector' => [
              'cvterm_id' => $cv_settings['BiologicalStatusOfAccessionCode'],
              'is_not' => FALSE,
            ],
            'field_type' => 'cvterm',
          ],
          // New slightly elaborated post-processed.
          'biologicalStatusOfAccessionCode' => function (
            $data_type,
            $stock,
            $field_name,
            $op,
            $values = NULL
          ) {
            // Caching status codes to avoid unnecessary calls to CVTerm table.
            static $status_codes_cache;
            if (!isset($status_codes_cache)) {
              $status_codes_cache = [];
            }
            // Default to NULL value == empty string unlees proven otherwise.
            $biological_status_of_accession_code = '';
            switch ($op) {
              case NULL:
                $biological_status_of_accession_code = ['read' => TRUE];
                break;

              case 'read':
                try {
                  $status_code_cvterm_id = brapi_get_field(
                    $data_type,
                    $stock,
                    'biologicalStatusOfAccessionCodeCVTerm'
                  );

                  // Check for cached status code.
                  if (in_array($status_code_cvterm_id, $status_codes_cache)) {
                    $biological_status_of_accession_code = $status_codes_cache[$status_code_cvterm_id];
                  }
                  else {
                    $status_code_cvterm = chado_generate_var(
                      'cvterm',
                      ['cvterm_id' => $status_code_cvterm_id]
                    );
                    $status_code_name = ($status_code_cvterm ? $status_code_cvterm->name : '');
                    $biological_status_of_accession_code = ($status_code_name ? intval(substr($status_code_name, 0, 3)) : '');
                    $status_codes_cache[$status_code_cvterm_id] = $biological_status_of_accession_code;
                  }
                }
                catch (Exception $e) {
                  // Do nothing.
                  watchdog('brapi_get_field(germplasm.biologicalStatusOfAccessionCode)', $e->getMessage(), [], WATCHDOG_WARNING);
                }
                break;

              default:
            }
            return $biological_status_of_accession_code;
          },
          'commonCropNameOrganism' => [
            'table' => 'organism',
            'foreign_key' => 'organism_id',
            'object_key' => 'organism_id',
            'column' => 'common_name',
            'selector' => [],
            'field_type' => 'string',
          ],
          'commonCropNameProp' => [
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => [
              'type_id' => $cv_settings['commonCropName'],
            ],
            'field_type' => 'string',
          ],
          'countryOfOriginCodeCVTerm' => [
            'table' => 'stock_cvterm',
            'column' => 'cvterm_id',
            'selector' => [
              'cvterm_id' => $cv_settings['countryOfOriginCode'],
              'is_not' => FALSE,
            ],
            'field_type' => 'cvterm',
          ],
          'countryOfOriginCodeProp' => [
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => [
              'type_id' => $cv_settings['countryOfOriginCode'],
            ],
            'field_type' => 'string',
          ],
          'donorsProp' => [
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => [
              'type_id' => $cv_settings['donor institute'],
            ],
            'field_type' => 'string',
            'return_array' => TRUE,
          ],
          'femaleParentStockId' => [
            'table' => 'stock_relationship',
            'column' => 'subject_id',
            'foreign_key' => 'object_id',
            'selector' => [
              'type_id' => $cv_settings['FemaleParentOf'],
            ],
            'field_type' => 'int',
          ],
          // Genus from chado.organism.
          'genusOrganism' => [
            'table' => 'organism',
            'foreign_key' => 'organism_id',
            'object_key' => 'organism_id',
            'column' => 'genus',
            'selector' => [],
            'field_type' => 'string',
          ],
          'genusProp' => [
            // Genus from MCPD.
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => [
              'type_id' => $cv_settings['genus'],
            ],
            'field_type' => 'string',
          ],
          'germplasmName' => [
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => [
              'type_id' => $cv_settings['germplasmName'],
            ],
            'field_type' => 'string',
          ],
          'germplasmPUI' => [
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => [
              'type_id' => $cv_settings['persistent unique identifier'],
            ],
            'field_type' => 'string',
          ],
          'instituteCodeProp' => [
            // From MCPD cvterm 'institute code'.
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => [
              'type_id' => $cv_settings['instituteCode'],
            ],
            'field_type' => 'string',
          ],
          'instituteNameProp' => [
            // Note: the use of MCPD 'institute code' should be privileged
            // against the use of 'Breeding institute name'. If 'institute code'
            // is set, then this field should be left empty.
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => [
              'type_id' => $cv_settings['instituteName'],
            ],
            'field_type' => 'string',
          ],
          'maleParentStockId' => [
            'table' => 'stock_relationship',
            'column' => 'subject_id',
            'foreign_key' => 'object_id',
            'selector' => [
              'type_id' => $cv_settings['MaleParentOf'],
            ],
            'field_type' => 'int',
          ],
          'seedSource' => [
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => [
              'type_id' => $cv_settings['SeedSource'],
            ],
            'field_type' => 'string',
          ],
          'speciesAuthorityProp' => [
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => [
              'type_id' => $cv_settings['speciesAuthority'],
            ],
            'field_type' => 'string',
          ],
          'speciesOrganism' => [
            // Species from chado.organism.
            'table' => 'organism',
            'foreign_key' => 'organism_id',
            'object_key' => 'organism_id',
            'column' => 'species',
            'selector' => [],
            'field_type' => 'string',
          ],
          'speciesInfra' => [
            // Species and subspecies from chado.organism.
            'table' => 'organism',
            'foreign_key' => 'organism_id',
            'object_key' => 'organism_id',
            'column' => 'species',
            'selector' => [],
            'field_type' => 'string',
          ],
          'speciesProp' => [
            // Species from MCPD.
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => [
              'type_id' => $cv_settings['species'],
            ],
            'field_type' => 'string',
          ],
          'subtaxaAuthorityProp' => [
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => [
              'type_id' => $cv_settings['subtaxaAuthority'],
            ],
            'field_type' => 'string',
          ],
          'subtaxaProp' => [
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => [
              'type_id' => $cv_settings['subtaxa'],
            ],
            'field_type' => 'string',
          ],
          'synonyms' => [
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => [
              'type_id' => $cv_settings['GermplasmSynonym'],
            ],
            'field_type' => 'string',
            'return_array' => TRUE,
            'use_ranking' => TRUE,
          ],
          'typeOfGermplasmStorageCode' => [
            'table' => 'stock_cvterm',
            'column' => 'cvterm_id',
            'selector' => [
              'cvterm_id' => $cv_settings['TypeOfGermplasmStorageCode'],
              'is_not' => FALSE,
            ],
            'field_type' => 'cvterm',
            'return_array' => TRUE,
          ],

          // Function fields.
          'donors' => function ($data_type, $stock, $field_name, $op, $values = NULL) use ($cv_settings) {
            $donors = NULL;
            switch ($op) {
              case NULL:
                $donors = ['read' => TRUE];
                break;

              case 'read':
                $sql_query = '
                  SELECT
                    sp_donor.value AS "donorInstituteCode",
                    sp_dan.value AS "donorAccessionNumber",
                    sp_dgpui.value AS "donorGermplasmPUI",
                    sp_donor.rank AS "rank"
                  FROM
                    {stock} s
                    JOIN {stockprop} sp_donor ON sp_donor.stock_id = s.stock_id AND sp_donor.type_id = :donor
                    LEFT JOIN {stockprop} sp_dan ON sp_dan.stock_id = s.stock_id AND sp_dan.type_id = :dan AND sp_dan.rank = sp_donor.rank
                    LEFT JOIN {stockprop} sp_dgpui ON sp_dgpui.stock_id = s.stock_id AND sp_dgpui.type_id = :dpui AND sp_dgpui.rank = sp_donor.rank
                  WHERE
                   s.stock_id = :stock_id
                  ORDER BY sp_donor.rank ASC
                ;';
                $filter_values = [
                  ':stock_id' => $stock->stock_id,
                  ':dan' => $cv_settings['donor accession number'],
                  ':donor' => $cv_settings['donor institute'],
                  ':dpui' => $cv_settings['DonorGermplasmPUI'],
                ];
                $prevloc_result = chado_query($sql_query, $filter_values);
                if ($prevloc_result) {
                  $donors = [];
                  while ($prevloc = $prevloc_result->fetchAssoc()) {
                    $donors[$prevloc['rank']] = [
                      'donorAccessionNumber' => $prevloc['donorAccessionNumber'],
                      'donorInstituteCode' => $prevloc['donorInstituteCode'],
                      'germplasmPUI' => $prevloc['donorGermplasmPUI'],
                    ];
                  }
                }
                break;

              default:
            }
            return $donors;
          },
          'germplasmPUIDrupalURL' => function ($data_type, $stock, $field_name, $op, $values = NULL) {
            global $base_url;
            $germplasm_pui = '';
            switch ($op) {
              case NULL:
                $germplasm_pui = ['read' => TRUE];
                break;

              case 'read':
                if ($nid = chado_get_nid_from_id('stock', $stock->stock_id)) {
                  $germplasm_pui = $base_url . '/' . drupal_get_path_alias('node/' . $nid);
                }
                break;

              default:
            }
            return $germplasm_pui;
          },
          'instituteCodeStockCollection' => function ($data_type, $stock, $field_name, $op, $values = NULL) {
            // If not specified through MCPD, uses stockcollection.uniquename.
            $institute_code_stock_collection = NULL;
            switch ($op) {
              case NULL:
                $institute_code_stock_collection = ['read' => TRUE];
                break;

              case 'read':
                $stock = chado_expand_var($stock, 'table', 'stockcollection_stock');
                if (isset($stock->stockcollection_stock)
                    && isset($stock->stockcollection_stock->stockcollection_id)) {
                  $institute_code_stock_collection = $stock->stockcollection_stock->stockcollection_id->uniquename;
                }
                break;

              default:
            }
            return $institute_code_stock_collection;
          },
          'instituteNameStockCollection' => function ($data_type, $stock, $field_name, $op, $values = NULL) {
            // If not specified through MCPD, uses stockcollection.name.
            $institute_name_stock_collection = NULL;
            switch ($op) {
              case NULL:
                $institute_name_stock_collection = ['read' => TRUE];
                break;

              case 'read':
                $stock = chado_expand_var($stock, 'table', 'stockcollection_stock');
                if (isset($stock->stockcollection_stock)
                    && isset($stock->stockcollection_stock->stockcollection_id)) {
                  $institute_name_stock_collection = $stock->stockcollection_stock->stockcollection_id->name;
                }
                break;

              default:
            }
            return $institute_name_stock_collection;
          },
          'parent1DbId' => function ($data_type, $stock, $field_name, $op, $values = NULL) {
            $parent1_db_id = '';
            switch ($op) {
              case NULL:
                $parent1_db_id = ['read' => TRUE];
                break;

              case 'read':
                $parent_stock_id = brapi_get_field(
                  $data_type,
                  $stock,
                  'femaleParentStockId'
                );
                $parent = chado_generate_var(
                  'stock',
                  ['stock_id' => $parent_stock_id]
                );
                $parent1_db_id = ($parent ? $parent->uniquename : '');

              default:
            }
            return $parent1_db_id;
          },
          'parent2DbId' => function ($data_type, $stock, $field_name, $op, $values = NULL) {
            $parent2_db_id = '';
            switch ($op) {
              case NULL:
                $parent2_db_id = ['read' => TRUE];
                break;

              case 'read':
                $parent_stock_id = brapi_get_field(
                  $data_type,
                  $stock,
                  'maleParentStockId'
                );
                $parent = chado_generate_var(
                  'stock',
                  ['stock_id' => $parent_stock_id]
                );
                $parent2_db_id = ($parent ? $parent->uniquename : '');
                break;

              default:
            }

            return $parent2_db_id;
          },
          'pedigree' => function ($data_type, $stock, $field_name, $op, $values = NULL) {
            $pedigree = '';
            switch ($op) {
              case NULL:
                $pedigree = ['read' => TRUE];
                break;

              case 'read':
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
                else {
                  $pedigree = $female_parent . $male_parent;
                }
                break;

              default:
            }
            return $pedigree;
          },
          'speciesFunc' => $organism_info_func,
          'speciesAuthorityFunc' => $organism_info_func,
          'subtaxaFunc' => $organism_info_func,
          'subtaxaAuthorityFunc' => $organism_info_func,
          'speciesInfraFunc' => $subtaxon_info_func,
          'speciesInfraAuthorityFunc' => $subtaxon_info_func,
          'subtaxaInfraFunc' => $subtaxon_info_func,
          'subtaxaInfraAuthorityFunc' => $subtaxon_info_func,
          'taxonId' => function ($data_type, $stock, $field_name, $op, $values = NULL) {
            $taxon_id = NULL;
            switch ($op) {
              case NULL:
                $taxon_id = ['read' => TRUE];
                break;

              case 'read':
                $taxon_id = NULL;
                // @todo Implement taxonId management.
                break;

              default:
            }
            return $taxon_id;
          },
        ],
      ),
      'sample' => array(
        'table' => 'stock',
        'identifier' => 'stock_id',
        'accession' => 'uniquename',
        'selector' => array(
          'type_id' => $cv_settings['Sample'],
        ),
        'fields' => array(
          // @todo list:
          // Table fields.
          'sampleDbId' => 'uniquename',

          // Aliases.
          // Table joins.
          'experimentId' => array(
            'table' => 'nd_experiment_stock',
            'column' => 'nd_experiment_id',
            'selector' => array(),
            'field_type' => 'string',
          ),
          'notes' => array(
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['Notes'],
            ),
            'field_type' => 'string',
          ),
          'plantDbId' => array(
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['PlantDbId'],
            ),
            'field_type' => 'string',
          ),
          'sampleType' => array(
            'table' => 'cvterm',
            'column' => 'name',
            'object_key' => 'type_id',
            'foreign_key' => 'cvterm_id',
            'selector' => array(),
            'field_type' => 'string',
          ),
          'sourceStockId' => array(
            'table' => 'stock_relationship',
            'column' => 'object_id',
            'foreign_key' => 'subject_id',
            'selector' => array(
              'type_id' => $cv_settings['SampleOf'],
            ),
            'field_type' => 'int',
          ),
          'tissueType' => array(
            'table' => 'stock_cvterm',
            'column' => 'cvterm_id',
            'selector' => array(
              'cvterm_id' => $cv_settings['TypeOfTissue'],
              'is_not' => FALSE,
            ),
            'field_type' => 'cvterm',
          ),

          // Function fields.
          'germplasmDbId' => function ($data_type, $stock, $field_name, $op, $values = NULL) {
            $source_db_id = '';
            switch ($op) {
              case NULL:
                $source_db_id = array('read' => TRUE);
                break;

              case 'read':
                $parent_stock_id = brapi_get_field(
                  $data_type,
                  $stock,
                  'sourceStockId'
                );
                $parent = chado_generate_var(
                  'stock',
                  array('stock_id' => $parent_stock_id)
                );
                $source_db_id = ($parent ? $parent->uniquename : '');

              default:
            }
            return $source_db_id;
          },
          'observationUnitDbId' => function ($data_type, $stock, $field_name, $op, $values = NULL) {
            $observation = NULL;
            switch ($op) {
              case NULL:
                $observation = array('read' => TRUE);
                break;

              case 'read':
                $observation = NULL;

              default:
            }
            return $observation;
          },
          'plateDbId' => function ($data_type, $stock, $field_name, $op, $values = NULL) use ($cv_settings) {
            $plate_db_id = NULL;
            switch ($op) {
              case NULL:
                $plate_db_id = array('read' => TRUE);
                break;

              case 'read':
                $experiment_id = brapi_get_field(
                  $data_type,
                  $stock,
                  'experimentId'
                );
                if ($experimentprop = chado_generate_var(
                  'nd_experimentprop',
                  array(
                    'nd_experiment_id' => $experiment_id,
                    'type_id' => $cv_settings['PlateDbId'],
                  )
                )) {
                  $plate_db_id = $experimentprop->value;
                }

              default:
            }
            return $plate_db_id;
          },
          'plateIndex' => function ($data_type, $stock, $field_name, $op, $values = NULL) use ($cv_settings) {
            $plate_index = NULL;
            switch ($op) {
              case NULL:
                $plate_index = array('read' => TRUE);
                break;

              case 'read':
                $experiment_id = brapi_get_field(
                  $data_type,
                  $stock,
                  'experimentId'
                );
                if ($experimentprop = chado_generate_var(
                  'nd_experimentprop',
                  array(
                    'nd_experiment_id' => $experiment_id,
                    'type_id' => $cv_settings['PlateIndex'],
                  )
                )) {
                  $plate_index = $experimentprop->value;
                }

              default:
            }
            return $plate_index;
          },
          'plotDbId' => function ($data_type, $stock, $field_name, $op, $values = NULL) use ($cv_settings) {
            $plot_db_id = NULL;
            switch ($op) {
              case NULL:
                $plot_db_id = array('read' => TRUE);
                break;

              case 'read':
                $experiment_id = brapi_get_field(
                  $data_type,
                  $stock,
                  'experimentId'
                );
                if ($experimentprop = chado_generate_var(
                  'nd_experimentprop',
                  array(
                    'nd_experiment_id' => $experiment_id,
                    'type_id' => $cv_settings['PlotDbId'],
                  )
                )) {
                  $plot_db_id = $experimentprop->value;
                }

              default:
            }
            return $plot_db_id;
          },
          'sampleTimestamp' => function ($data_type, $stock, $field_name, $op, $values = NULL) use ($cv_settings) {
            $sample_timestamp = NULL;
            switch ($op) {
              case NULL:
                $sample_timestamp = array('read' => TRUE);
                break;

              case 'read':
                $experiment_id = brapi_get_field(
                  $data_type,
                  $stock,
                  'experimentId'
                );
                $experimentprop = chado_generate_var(
                  'nd_experimentprop',
                  array(
                    'nd_experiment_id' => $experiment_id,
                    'type_id' => $cv_settings['SampleTimestamp'],
                  )
                );
                if ($experimentprop && $experimentprop->value) {
                  $sample_timestamp = brapi_parse_date($experimentprop->value);
                }

              default:
            }
            return $sample_timestamp;
          },
          'studyDbId' => function ($data_type, $stock, $field_name, $op, $values = NULL) use ($cv_settings) {
            $study_db_id = NULL;
            switch ($op) {
              case NULL:
                $study_db_id = array('read' => TRUE);
                break;

              case 'read':
                $experiment_id = brapi_get_field(
                  $data_type,
                  $stock,
                  'experimentId'
                );
                if ($experimentprop = chado_generate_var(
                  'nd_experimentprop',
                  array(
                    'nd_experiment_id' => $experiment_id,
                    'type_id' => $cv_settings['Study'],
                  )
                )) {
                  $study_db_id = $experimentprop->value;
                }

              default:
            }
            return $study_db_id;
          },
          'takenBy' => function ($data_type, $stock, $field_name, $op, $values = NULL) use ($cv_settings) {
            $contact = NULL;
            switch ($op) {
              case NULL:
                $contact = array('read' => TRUE);
                break;

              case 'read':
                $experiment_id = brapi_get_field(
                  $data_type,
                  $stock,
                  'experimentId'
                );
                if ($experiment = chado_generate_var(
                  'nd_experiment',
                  array(
                    'nd_experiment_id' => $experiment_id,
                  )
                )) {
                  $experiment = chado_expand_var($experiment, 'table', 'nd_experiment_contact');
                  $contact = $experiment->nd_experiment_contact->contact_id->name;
                }

              default:
            }
            return $contact;
          },
        ),
      ),
      'marker' => array(
        'table' => 'feature',
        'identifier' => 'feature_id',
        'accession' => 'uniquename',
        'selector' => array(
          'type_id' => $cv_settings['MarkerType'],
        ),
        'fields' => array(
          // Table fields.
          'defaultDisplayName' => 'name',
          'markerDbId' => 'uniquename',
          'residues' => 'residues',

          // Aliases.
          // Table joins.
          'synonyms' => array(
            'table' => 'featureprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['MarkerSynonym'],
            ),
            'field_type' => 'string',
          ),
          'type' => array(
            'table' => 'cvterm',
            'foreign_key' => 'cvterm_id',
            'object_key' => 'type_id',
            'column' => 'name',
            'selector' => array(),
            'field_type' => 'string',
          ),

          // Function fields.
          'analysisMethods' => function ($data_type, $feature, $field_name, $op, $values = NULL) {
            $analysis = array();
            switch ($op) {
              case NULL:
                $analysis = array(
                  'read' => TRUE,
                  'filter' => TRUE,
                );
                break;

              case 'read':
                $feature = chado_expand_var($feature, 'table', 'analysisfeature');
                $analysisfeatures = $feature->analysisfeature;
                if ($analysisfeatures) {
                  if (is_array($analysisfeatures)) {
                    foreach ($analysisfeatures as $analysisfeature) {
                      $analysis[] = $analysisfeature->analysis_id->name;
                    }
                  }
                  else {
                    $analysis[] = $analysisfeatures->analysis_id->name;
                  }
                }
                break;

              case 'filter':
                // Get possible values as an array.
                if (is_array($values)) {
                  $values = implode(',', $values);
                }
                $values = explode(',', $values);
                // Check each object.
                foreach ($feature as $tocheck) {
                  $analysis_methods = brapi_get_field(
                    $data_type,
                    $tocheck,
                    'analysisMethods'
                  );
                  if (array_intersect($analysis_methods, $values)) {
                    $analysis[] = $tocheck;
                  }
                }
                break;

              default:
            }

            return $analysis;
          },
          'refAlt' => function ($data_type, $feature, $field_name, $op, $values = NULL) {
            $ref_alt = array();
            switch ($op) {
              case NULL:
                $ref_alt = array('read' => TRUE);
                break;

              case 'read':
                $residues = brapi_get_field(
                  $data_type,
                  $feature,
                  'residues'
                );
                // @todo: replace UPAC codes by corresponding list of
                // nucleotides.
                $ref_alt = array($residues);
                break;

              default:
            }
            return $ref_alt;
          },
        ),
      ),
      'allelematrix' => array(
        // @code
        // 'table' => 'xxx',
        // 'identifier' => 'xx_id',
        // 'accession' => 'uniquename',
        // 'fields' => array(
        //  // Table fields.
        //  // Aliases.
        //  // Table joins.
        //  // Function fields.
        //   'xxxDbId' => 'uniquename',
        //   'defaultDisplayName' => 'name',
        //   'type' => array(
        //     'table' => 'cvterm',
        //     'foreign_key' => 'cvterm_id',
        //     'object_key' => 'type_id',
        //     'column' => 'name',
        //     'selector' => array(),
        //     'field_type' => 'string',
        //   ),
        // ),
        // @endcode
      ),
      'trait' => array(
        'table'      => 'cvterm',
        'identifier' => 'cvterm_id',
        'accession'  => 'cvterm_id',
        'selector' => (
          isset($storage_options['traits']) ?
          brapi_v1_get_cvterm_selector($storage_options['traits'])
          : NULL
        ),
        'fields' => array(
          'defaultDisplayName' => 'name',
          'description'        => 'definition',
          'name'               => 'name',
          'traitDbId'          => 'cvterm_id',
          'traitName'          => 'name',
          'defaultValue'       => function ($data_type, $cvterm, $field_name, $op, $values = NULL) use ($cv_settings) {
            $default_value = NULL;
            switch ($op) {
              case NULL:
                $default_value = array('read' => TRUE);
                break;

              case 'read':
                $default_value = NULL;
                break;

              default:
            }
            return $default_value;
          },
          'observationVariables' => function (
            $data_type,
            $cvterm,
            $field_name,
            $op,
            $values = NULL
          ) {
            $observationVariables = array();
            switch ($op) {
              case NULL:
                $observationVariables = array('read' => TRUE);
                break;

              case 'read':
                $sql_query = '
                  SELECT db.name || \':\' || dbx.accession AS "name"
                  FROM {cvterm} v
                    JOIN {dbxref} dbx USING (dbxref_id)
                    JOIN {db} db USING (db_id)
                    JOIN {cvterm_relationship} vr ON vr.subject_id=v.cvterm_id
                    JOIN {cvterm} t ON t.cvterm_id=vr.type_id
                      AND t.name = :variable_of
                  WHERE
                    vr.object_id = :cvterm_id
                ;';
                $filter_values = array(
                  ':cvterm_id' => $cvterm->cvterm_id,
                  ':variable_of' => 'variable_of',
                );
                $var_result = chado_query($sql_query, $filter_values);
                if ($var_result) {
                  while ($variable = $var_result->fetchAssoc()) {
                    $observationVariables[] = $variable['name'];
                  }
                }
                break;

              default:
            }
            return $observationVariables;
          },
          'traitId' => function ($data_type, $cvterm, $field_name, $op, $values = NULL) use ($cv_settings) {
            $trait_id = NULL;
            switch ($op) {
              case NULL:
                $trait_id = array('read' => TRUE);
                break;

              case 'read':
                $sql_query = '
                  SELECT db.name || \':\' || dbx.accession AS "trait_id"
                  FROM {cvterm} cvt
                    JOIN {dbxref} dbx USING (dbxref_id)
                    JOIN {db} USING (db_id)
                  WHERE cvterm_id = :cvterm_id
                ;';
                $filter_values = array(
                  ':cvterm_id' => $cvterm->cvterm_id,
                );
                $result = chado_query($sql_query, $filter_values);
                if ($result) {
                  $trait_id = $result->fetchAssoc()['trait_id'];
                }
                break;

              default:
            }
            return $trait_id;
          },
        ),
      ),
      'study'        => array(
        // @code
        // 'table' => 'xxx',
        // 'identifier' => 'xx_id',
        // 'accession' => 'uniquename',
        // 'fields' => array(
        //  // Table fields.
        //  // Aliases.
        //  // Table joins.
        //  // Function fields.
        //   'xxxDbId' => 'uniquename',
        //   'defaultDisplayName' => 'name',
        //   'type' => array(
        //     'table' => 'cvterm',
        //     'foreign_key' => 'cvterm_id',
        //     'object_key' => 'type_id',
        //     'column' => 'name',
        //     'selector' => array(),
        //     'field_type' => 'string',
        //   ),
        // ),
        // @endcode
      ),
      'attribute'    => array(
        // @code
        // 'table' => 'xxx',
        // 'identifier' => 'xx_id',
        // 'accession' => 'uniquename',
        // 'fields' => array(
        //  // Table fields.
        //  // Aliases.
        //  // Table joins.
        //  // Function fields.
        //   'xxxDbId' => 'uniquename',
        //   'defaultDisplayName' => 'name',
        //   'type' => array(
        //     'table' => 'cvterm',
        //     'foreign_key' => 'cvterm_id',
        //     'object_key' => 'type_id',
        //     'column' => 'name',
        //     'selector' => array(),
        //     'field_type' => 'string',
        //   ),
        // ),
        // @endcode
      ),
      'program'      => array(
        'table' => 'project',
        'identifier' => 'project_id',
        'accession' => 'project_id',
        'fields' => array(
          // Table fields.
          'abbreviation' => 'name',
          'name' => 'name',
          'objective' => 'description',
          'programDbId' => 'project_id',

          // Aliases.
          // Table joins.
          'contact_id' => array(
            'table' => 'project_contact',
            'column' => 'contact_id',
            'selector' => array(),
            'field_type' => 'int',
            'return_array' => TRUE,
            'use_ranking' => FALSE,
          ),

          // Function fields.
          'leadPerson' => function ($data_type, $project, $field_name, $op, $values = NULL) use ($cv_settings) {
            $lead_contact = NULL;
            switch ($op) {
              case NULL:
                $lead_contact = array('read' => TRUE);
                break;

              case 'read':
                $sql_query = '
                  SELECT c.name
                  FROM {contact} c
                    JOIN {project_contact} pc USING (contact_id)
                  WHERE
                    pc.project_id = :project_id
                    AND c.type_id = :lead_person_type_id
                ;';
                $filter_values = array(
                  ':project_id' => $project->project_id,
                  ':lead_person_type_id' => $cv_settings['LeadPerson'],
                );
                $contact_result = chado_query($sql_query, $filter_values);
                if ($contact_result) {
                  $contact = $contact_result->fetchAssoc();
                  $lead_contact = $contact['name'];
                }
                else {
                  $lead_contact = NULL;
                }
                break;

              default:
            }
            return $lead_contact;
          },
        ),
      ),
      'studyType'    => array(
        // @code
        // 'table' => 'xxx',
        // 'identifier' => 'xx_id',
        // 'accession' => 'uniquename',
        // 'fields' => array(
        //  // Table fields.
        //  // Aliases.
        //  // Table joins.
        //  // Function fields.
        //   'xxxDbId' => 'uniquename',
        //   'defaultDisplayName' => 'name',
        //   'type' => array(
        //     'table' => 'cvterm',
        //     'foreign_key' => 'cvterm_id',
        //     'object_key' => 'type_id',
        //     'column' => 'name',
        //     'selector' => array(),
        //     'field_type' => 'string',
        //   ),
        // ),
        // @endcode
      ),
      'phenotype'    => array(
        // @code
        // 'table' => 'xxx',
        // 'identifier' => 'xx_id',
        // 'accession' => 'uniquename',
        // 'fields' => array(
        //  // Table fields.
        //  // Aliases.
        //  // Table joins.
        //  // Function fields.
        //   'xxxDbId' => 'uniquename',
        //   'defaultDisplayName' => 'name',
        //   'type' => array(
        //     'table' => 'cvterm',
        //     'foreign_key' => 'cvterm_id',
        //     'object_key' => 'type_id',
        //     'column' => 'name',
        //     'selector' => array(),
        //     'field_type' => 'string',
        //   ),
        // ),
        // @endcode
      ),
      'variable'     => array(
        'table' => 'cvterm',
        'identifier' => 'cvterm_id',
        'accession' => 'name',
        'fields' => array(
          // Table fields.
          'name' => 'name',
          'objective' => 'definition',
          'variableDbId' => 'cvterm_id',

          // Aliases.
          'abbreviation' => array(
            'alias_for' => 'name',
          ),
          'observationVariableDbId' => array(
            'alias_for' => 'variableDbId',
          ),

          // Table joins.
          // Function fields.
          'leadPerson' => function ($data_type, $cvterm, $field_name, $op, $values = NULL) use ($cv_settings) {
            $lead_contact = NULL;
            switch ($op) {
              case NULL:
                $lead_contact = array('read' => TRUE);
                break;

              case 'read':
                $sql_query = '
                  SELECT c.name
                  FROM {contact} c
                    JOIN {project_contact} pc USING (contact_id)
                  WHERE
                    pc.project_id = :project_id
                    AND c.type_id = :lead_person_type_id
                ;';
                $filter_values = array(
                  ':project_id' => $project->project_id,
                  ':lead_person_type_id' => $cv_settings['LeadPerson'],
                );
                $contact_result = chado_query($sql_query, $filter_values);
                if ($contact_result) {
                  $contact = $contact_result->fetchAssoc();
                  $lead_contact = $contact['name'];
                }
                else {
                  $lead_contact = NULL;
                }
                break;

              default:
            }
            return $lead_contact;
          },
        ),
      ),
      'map'          => array(
        // @code
        // 'table' => 'xxx',
        // 'identifier' => 'xx_id',
        // 'accession' => 'uniquename',
        // 'fields' => array(
        //  // Table fields.
        //  // Aliases.
        //  // Table joins.
        //  // Function fields.
        //   'xxxDbId' => 'uniquename',
        //   'defaultDisplayName' => 'name',
        //   'type' => array(
        //     'table' => 'cvterm',
        //     'foreign_key' => 'cvterm_id',
        //     'object_key' => 'type_id',
        //     'column' => 'name',
        //     'selector' => array(),
        //     'field_type' => 'string',
        //   ),
        // ),
        // @endcode
      ),
      'observation'  => array(
        // @code
        // 'table' => 'xxx',
        // 'identifier' => 'xx_id',
        // 'accession' => 'uniquename',
        // 'fields' => array(
        //  // Table fields.
        //  // Aliases.
        //  // Table joins.
        //  // Function fields.
        //   'xxxDbId' => 'uniquename',
        //   'defaultDisplayName' => 'name',
        //   'type' => array(
        //     'table' => 'cvterm',
        //     'foreign_key' => 'cvterm_id',
        //     'object_key' => 'type_id',
        //     'column' => 'name',
        //     'selector' => array(),
        //     'field_type' => 'string',
        //   ),
        // ),
        // @endcode
      ),
      'location'     => array(
        'table' => 'nd_geolocation',
        'identifier' => 'nd_geolocation_id',
        'accession' => 'nd_geolocation_id',
        'fields' => array(
          // Table fields.
          'altitude' => 'altitude',
          'latitude' => 'latitude',
          'locationDbId' => 'nd_geolocation_id',
          'longitude' => 'longitude',
          'name' => 'description',

          // Aliases.
          // Table joins.
          'abbreviation' => array(
            'table' => 'nd_geolocationprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['LocationAbbreviation'],
            ),
            'field_type' => 'string',
          ),
          'countryCode' => array(
            'table' => 'nd_geolocationprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['CountryCode'],
            ),
            'field_type' => 'string',
          ),
          'countryName' => array(
            'table' => 'nd_geolocationprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['CountryName'],
            ),
            'field_type' => 'string',
          ),
          'instituteAddress' => array(
            'table' => 'nd_geolocationprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['InstituteAddress'],
            ),
            'field_type' => 'string',
          ),
          'instituteName' => array(
            'table' => 'nd_geolocationprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['InstituteName'],
            ),
            'field_type' => 'string',
          ),
          'locationType' => array(
            'table' => 'nd_geolocationprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['LocationType'],
            ),
            'field_type' => 'string',
          ),

          // Function fields.
          'additionalInfo' => function ($data_type, $geoloc, $field_name, $op, $values = NULL) use ($cv_settings) {
            $info = array();
            switch ($op) {
              case NULL:
                $info = array(
                  'read' => TRUE,
                );
                break;

              case 'read':
                $feature = chado_expand_var($geoloc, 'table', 'nd_geolocationprop');
                $geolocationprops = $geoloc->nd_geolocationprop;
                if (!$geolocationprops) {
                  $geolocationprops = array();
                }
                elseif (!is_array($geolocationprops)) {
                  $geolocationprops = array($geolocationprops);
                }
                $used_types = array(
                  $cv_settings['LocationType'],
                  $cv_settings['LocationAbbreviation'],
                  $cv_settings['CountryCode'],
                  $cv_settings['CountryName'],
                  $cv_settings['InstituteName'],
                  $cv_settings['InstituteAddress'],
                );
                foreach ($geolocationprops as $geolocationprop) {
                  // Make sure it is not a property already in use.
                  if (!in_array($geolocationprop->type_id->cvterm_id, $used_types)) {
                    $info[$geolocationprop->type_id->name] = $geolocationprop->value;
                  }
                }

                if (empty($info)) {
                  $info = NULL;
                }
                break;

              default:
            }

            return $info;
          },
        ),
      ),
    );

    // Alter default settings.
    // Germplasm fields...
    $germplasm_fields = &$brapi_data_mapping['germplasm']['fields'];

    // Set "germplasmPUI" settings.
    $use_specified_settings =
      !empty($storage_options['germplasm_pui'])
      && !empty($germplasm_fields[$storage_options['germplasm_pui']]);
    if ($use_specified_settings) {
      $germplasm_fields['germplasmPUI'] =
        &$germplasm_fields[$storage_options['germplasm_pui']];
    }

    // Set "common crop name" settings, default to prop.
    $use_specified_settings =
      !empty($storage_options['common_crop_name'])
      && !empty($germplasm_fields[$storage_options['common_crop_name']]);
    if ($use_specified_settings) {
      $germplasm_fields['commonCropName']['alias_for'] =
        $storage_options['common_crop_name'];
    }

    // Set "genus" settings, default to prop.
    $use_specified_settings =
      !empty($storage_options['genus'])
      && !empty($germplasm_fields[$storage_options['genus']]);
    if ($use_specified_settings) {
      $germplasm_fields['genus']['alias_for'] = $storage_options['genus'];
    }

    // Set "species" and associated settings, default to organism through
    // functions.
    $use_specified_settings =
      !empty($storage_options['species'])
      && !empty($germplasm_fields[$storage_options['species']]);
    if ($use_specified_settings) {
      switch ($storage_options['species']) {
        case 'speciesProp':
          $germplasm_fields['species']['alias_for'] = 'speciesProp';
          $germplasm_fields['speciesAuthority']['alias_for'] = 'speciesAuthorityProp';
          $germplasm_fields['subtaxa']['alias_for'] = 'subtaxaProp';
          $germplasm_fields['subtaxaAuthority']['alias_for'] = 'subtaxaAuthorityProp';
          break;

        case 'speciesOrganism':
          $germplasm_fields['species']['alias_for'] = 'speciesOrganism';
          $germplasm_fields['speciesAuthority']['alias_for'] = 'speciesAuthorityProp';
          $germplasm_fields['subtaxa']['alias_for'] = 'subtaxaProp';
          $germplasm_fields['subtaxaAuthority']['alias_for'] = 'subtaxaAuthorityProp';
          break;

        case 'speciesInfraFunc':
          $germplasm_fields['species']['alias_for'] = 'speciesInfraFunc';
          $germplasm_fields['speciesAuthority']['alias_for'] = 'speciesInfraAuthorityFunc';
          $germplasm_fields['subtaxa']['alias_for'] = 'subtaxaInfraFunc';
          $germplasm_fields['subtaxaAuthority']['alias_for'] = 'subtaxaInfraAuthorityFunc';
          break;

        default:
          // Through organism and phylonode tables.
          $germplasm_fields['species']['alias_for'] = 'speciesFunc';
          $germplasm_fields['speciesAuthority']['alias_for'] = 'speciesAuthorityFunc';
          $germplasm_fields['subtaxa']['alias_for'] = 'subtaxaFunc';
          $germplasm_fields['subtaxaAuthority']['alias_for'] = 'subtaxaAuthorityFunc';
          break;
      }
    }

    // Set "country of origin" settings, default to prop.
    $use_specified_settings =
      !empty($storage_options['country_of_origin_code'])
      && !empty($germplasm_fields[$storage_options['country_of_origin_code']]);
    if ($use_specified_settings) {
      $germplasm_fields['countryOfOriginCode']['alias_for'] =
        $storage_options['country_of_origin_code'];
    }

    // Set "institute code" settings, default to prop.
    $use_specified_settings =
      !empty($storage_options['institute_code'])
      && !empty($germplasm_fields[$storage_options['institute_code']]);
    if ($use_specified_settings) {
      $germplasm_fields['instituteCode']['alias_for'] =
        $storage_options['institute_code'];
    }

    // Set "institute name" settings, default to prop.
    $use_specified_settings =
      !empty($storage_options['institute_name'])
      && !empty($germplasm_fields[$storage_options['institute_name']]);
    if ($use_specified_settings) {
      $germplasm_fields['instituteName']['alias_for'] =
        $storage_options['institute_name'];
    }

    // BrAPI *DbId.
    if (isset($storage_options['dbid'])
        && ('chado_id' == $storage_options['dbid'])) {
      foreach (array_keys($brapi_data_mapping) as $data_type) {
        if (!empty($brapi_data_mapping[$data_type])) {
          $brapi_data_mapping[$data_type]['fields'][$data_type . 'DbId'] =
            $brapi_data_mapping[$data_type]['identifier'];
        }
      }
    }

    // Allow other modules to alter settings.
    drupal_alter('brapi_data_mapping', $brapi_data_mapping);
  }

  return $brapi_data_mapping;
}
