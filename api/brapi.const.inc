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
define('BRAPI_SERVICE_VERSION', '1.1');
define('BRAPI_SERVICE_PREFIX_V1', 'brapi/v1');
define('BRAPI_MCPD_VERSION', '2.1');
define('BRAPI_DB', 'breeding api');
define('BRAPI_CV', 'breeding api');
define('BRAPI_CV_SETTINGS', 'brapi_cv_settings');
define('BRAPI_EXAMPLE_SETTINGS', 'brapi_example_settings');
define('BRAPI_GERMPLASM_ATTR_CATEGORIES', 'brapi_germplasm_attr_categories');
define('BRAPI_GERMPLASM_ATTRIBUTES', 'brapi_germplasm_attributes');
define('BRAPI_AGGREGATION_OPTIONS', 'brapi_aggregation_options');
define('BRAPI_STORAGE_OPTIONS', 'brapi_storage_options');
define('BRAPI_DATE_FORMAT', 'brapi_date_format');
define('BRAPI_CUSTOM_DATE_FORMAT', 'brapi_custom_date_format');
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
    'CountryCode'      => 'Geo-location country code (nd_geolocationprop.type_id).',
    'CountryName'      => 'Geo-location country name (nd_geolocationprop.type_id).',
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
      'persistent unique identifier' => array('name' => 'persistent unique identifier'),
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
 *   - 'page callback': name of the function that returns the JSON output. If
 *     set to NULL, it will use 'callback versions' selected by
 *     'active version';
 *   - 'active version': label of the active implementation version of the call;
 *   - 'callback versions': names of the callbacks by version that can be used
 *     as 'page callback';
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
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_about_json',
        ),
        'aggregatable' => FALSE,
      ),
      'allelematrices'     => array(
        'title'          => 'Allele Matrices',
        'description'    => 'List allele matrices',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'aggregatable' => TRUE,
      ),
      'allelematrices-search'      => array(
        'title'          => 'MarkerProfile Allele Matrix',
        'description'    => 'Search allele matrices',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'aggregatable' => TRUE,
      ),
      'attributes' => array(
        'title'          => 'Germplasm Attribute List',
        'description'    => 'List attributes available',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_attributes_json',
        ),
        'aggregatable' => TRUE,
      ),
      'attributes/categories' => array(
        'title'          => 'Germplasm Attribute Category List',
        'description'    => 'List attribute categories available',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_categories_json',
        ),
        'aggregatable' => TRUE,
      ),
      'calls'      => array(
        'title'          => 'Call Search',
        'description'    => 'List supported Breeding API calls',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_call_json',
        ),
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
      'crops' => array(
        'title'          => 'Crops',
        'description'    => 'List supported crops',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_crops_json',
        ),
        'aggregatable' => TRUE,
      ),
      'germplasm' => array(
        'title'          => 'Germplasm',
        'description'    => 'Germplasm list',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_germplasm_json',
        ),
        'features'       => array('MCPD' => 'yes', 'MCPD-version' => 'V.' . BRAPI_MCPD_VERSION),
        'aggregatable' => TRUE,
      ),
      'germplasm/{germplasmDbId}' => array(
        'title'          => 'Germplasm Details',
        'description'    => 'Germplasm details by germplasmDbId',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'    => BRAPI_USE_PERMISSION,
          'POST'   => BRAPI_UPDATE_PERMISSION,
          'PUT'    => BRAPI_UPDATE_PERMISSION,
          'DELETE' => BRAPI_UPDATE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_germplasm_json',
        ),
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
      'germplasm/{germplasmDbId}/attributes' => array(
        'title'          => 'Germplasm Attributes',
        'description'    => 'Germplasm attribute values by germplasmDbId',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'    => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_germplasm_attributes_json',
        ),
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
      'germplasm/{germplasmDbId}/markerprofiles' => array(
        'title'          => 'Germplasm Markerprofiles',
        'description'    => 'Markerprofiles by germplasmDbId',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'    => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
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
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_germplasm_pedigree_json',
        ),
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
        'description'    => 'Germplasm search',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_germplasm_search_json',
        ),
        'features'       => array('MCPD' => 'yes', 'MCPD-version' => 'V.' . BRAPI_MCPD_VERSION),
        'aggregatable' => TRUE,
      ),
      'locations' => array(
        'title'          => 'Locations',
        'description'    => 'List Locations',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_location_json',
        ),
        'aggregatable' => TRUE,
      ),
      'locations/{locationDbId}' => array(
        'title'          => 'Locations Details',
        'description'    => 'Locations details by locationDbId',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
        ),
        'arguments'      => array(
          3 => array(
            'name' => 'locationDbId',
            'type' => 'string',
            'description' => 'correspond to a Chado nd_geolocation.nd_geolocation_id value',
            'required' => FALSE,
          ),
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_location_json',
        ),
        'aggregatable' => TRUE,
      ),
      'maps'      => array(
        'title'          => 'Genome Map',
        'description'    => 'Get list of maps',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'aggregatable' => TRUE,
      ),
      'maps/{mapDbId}' => array(
        'title'          => 'Genome Map Details',
        'description'    => 'Get map details',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'aggregatable' => TRUE,
      ),
      'maps/{mapDbId}/positions' => array(
        'title'          => 'Genome map data',
        'description'    => 'Get map data',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'aggregatable' => TRUE,
      ),
      'maps/{mapDbId}/positions/{linkageGroupName}' => array(
        'title'          => 'Genome Map Data by range on linkageGroup',
        'description'    => 'Get map data by range on linkageGroup',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'aggregatable' => TRUE,
      ),
      'markerprofiles'      => array(
        'title'          => 'Markerprofile Search',
        'description'    => 'List marker profiles',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'aggregatable' => TRUE,
      ),
      'markerprofiles/{markerprofileDbId}' => array(
        'title'          => 'Marker profiles',
        'description'    => 'Alleles by markerprofileDbId',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'aggregatable' => TRUE,
      ),
      'markers' => array(
        'title'          => 'Marker List',
        'description'    => 'List markers',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_marker_json',
        ),
        'aggregatable' => TRUE,
      ),
      'markers/{markerDbId}' => array(
        'title'          => 'Marker Details',
        'description'    => 'Markers details by markerDbId',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_marker_json',
        ),
        'arguments'      => array(
          3 => array(
            'name' => 'markerDbId',
            'type' => 'string',
            'description' => 'correspond to a Chado feature.uniquename value for feature type "genetic_marker"',
            'required' => FALSE,
          ),
        ),
        'aggregatable' => TRUE,
      ),
      'markers-search'      => array(
        'title'          => 'Markers Search',
        'description'    => 'Search markers',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_marker_search_json',
        ),
        'aggregatable' => TRUE,
      ),
      'observationlevels' => array(
        'title'          => 'Observation Level List',
        'description'    => 'List observation levels',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'aggregatable' => TRUE,
      ),
      'ontologies' => array(
        'title'          => 'Variable ontology list',
        'description'    => 'List ontologies',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'aggregatable' => TRUE,
      ),
      'phenotypes-search'      => array(
        'title'          => 'Phenotype Search',
        'description'    => 'Search phenotypes',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'aggregatable' => TRUE,
      ),
      'phenotypes'      => array(
        'title'          => 'Phenotype List',
        'description'    => 'List phenotypes',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'aggregatable' => TRUE,
      ),
      'phenotypes/{phenotypeDbId}' => array(
        'title'          => 'Phenotype Details',
        'description'    => 'Phenotypes details by phenotypeDbId',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'    => BRAPI_USE_PERMISSION,
          'POST'   => BRAPI_UPDATE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'arguments'      => array(
          3 => array(
            'name' => 'phenotypeDbId',
            'type' => 'string',
            'description' => 'n/a',
            'required' => FALSE,
          ),
        ),
        'aggregatable' => TRUE,
      ),
      'programs'      => array(
        'title'          => 'Program List',
        'description'    => 'List programs',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_program_json',
        ),
        'aggregatable' => TRUE,
      ),
      'programs/{programDbId}' => array(
        'title'          => 'Program Details',
        'description'    => 'Program details by programDbId',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_program_json',
        ),
        'arguments'      => array(
          3 => array(
            'name' => 'programDbId',
            'type' => 'string',
            'description' => 'correspond to a Chado project.project_id.',
            'required' => FALSE,
          ),
        ),
        'aggregatable' => TRUE,
      ),
      'programs-search'      => array(
        'title'          => 'Program Search',
        'description'    => 'Search programs',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_program_search_json',
        ),
        'aggregatable' => TRUE,
      ),
      'samples' => array(
        'title'          => 'Sample List',
        'description'    => 'Sample list call',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_sample_json',
        ),
        'aggregatable' => TRUE,
      ),
      'samples/{sampleDbId}' => array(
        'title'          => 'Sample Details',
        'description'    => 'Sample details by sampleDbId',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'    => BRAPI_USE_PERMISSION,
          'POST'   => BRAPI_UPDATE_PERMISSION,
          'PUT'    => BRAPI_UPDATE_PERMISSION,
          'DELETE' => BRAPI_UPDATE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_sample_json',
        ),
        'arguments'      => array(
          3 => array(
            'name' => 'sampleDbId',
            'type' => 'string',
            'description' => 'correspond to a Chado stock.uniquename value',
            'required' => FALSE,
          ),
        ),
        'aggregatable' => TRUE,
      ),
      'samples-search' => array(
        'title'          => 'Sample Search',
        'description'    => 'Search samples',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_sample_search_json',
        ),
        'aggregatable' => TRUE,
      ),
      'seasons'      => array(
        'title'          => 'Season List',
        'description'    => 'List seasons',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'aggregatable' => TRUE,
      ),
      'studies'      => array(
        'title'          => 'Study List',
        'description'    => 'List studies',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'aggregatable' => TRUE,
      ),
      'studies/{studyDbId}'      => array(
        'title'          => 'Study Details',
        'description'    => 'Studies details by studyDbId',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'arguments'      => array(
          3 => array(
            'name' => 'studyDbId',
            'type' => 'string',
            'description' => 'n/a',
            'required' => FALSE,
          ),
        ),
        'aggregatable' => TRUE,
      ),
      'studies/{studyDbId}/germplasm'      => array(
        'title'          => 'Study Germplasm Details',
        'description'    => 'Study germplasm details by studyDbId',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'arguments'      => array(
          3 => array(
            'name' => 'studyDbId',
            'type' => 'string',
            'description' => 'n/a',
            'required' => FALSE,
          ),
        ),
        'aggregatable' => TRUE,
      ),
      'studies/{studyDbId}/layout'      => array(
        'title'          => 'Plot Layout Details',
        'description'    => 'Retrieve plot layout details',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'arguments'      => array(
          3 => array(
            'name' => 'studyDbId',
            'type' => 'string',
            'description' => 'n/a',
            'required' => FALSE,
          ),
        ),
        'aggregatable' => TRUE,
      ),
      'studies/{studyDbId}/observations'      => array(
        'title'          => 'Observations Details',
        'description'    => 'Get observation units by observation variable by studyDbId',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'arguments'      => array(
          3 => array(
            'name' => 'studyDbId',
            'type' => 'string',
            'description' => 'n/a',
            'required' => FALSE,
          ),
        ),
        'aggregatable' => TRUE,
      ),
      'studies/{studyDbId}/observationunits'      => array(
        'title'          => 'Observation Units Details',
        'description'    => 'Get all observation units by studyDbId',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'arguments'      => array(
          3 => array(
            'name' => 'studyDbId',
            'type' => 'string',
            'description' => 'n/a',
            'required' => FALSE,
          ),
        ),
        'aggregatable' => TRUE,
      ),
      'studies/{studyDbId}/observationvariables'      => array(
        'title'          => 'Study Observation Variables',
        'description'    => 'Get observation variables by studyDbId',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'arguments'      => array(
          3 => array(
            'name' => 'studyDbId',
            'type' => 'string',
            'description' => 'n/a',
            'required' => FALSE,
          ),
        ),
        'aggregatable' => TRUE,
      ),
      'studies/{studyDbId}/table'      => array(
        'title'          => 'Study Observation Units as a Table',
        'description'    => 'Retrieve study observation units as table',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'arguments'      => array(
          3 => array(
            'name' => 'studyDbId',
            'type' => 'string',
            'description' => 'n/a',
            'required' => FALSE,
          ),
        ),
        'aggregatable' => TRUE,
      ),
      'studies-search'      => array(
        'title'          => 'Study search',
        'description'    => 'Search studies',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'aggregatable' => TRUE,
      ),
      'studytypes'      => array(
        'title'          => 'Study Type List',
        'description'    => 'List study types',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
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
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_authentication_json',
        ),
        'aggregatable' => FALSE,
      ),
      'traits'      => array(
        'title'          => 'Trait List',
        'description'    => 'List all traits',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'aggregatable' => TRUE,
      ),
      'traits/{traitDbId}' => array(
        'title'          => 'Trait Details',
        'description'    => 'Trait details by traitDbId',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'arguments'      => array(
          3 => array(
            'name' => 'traitDbId',
            'type' => 'string',
            'description' => 'n/a',
            'required' => FALSE,
          ),
        ),
        'aggregatable' => TRUE,
      ),
      'trials'      => array(
        'title'          => 'Trial List',
        'description'    => 'Get list of trial summaries',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'aggregatable' => TRUE,
      ),
      'trials/{trialDbId}' => array(
        'title'          => 'Trial Details',
        'description'    => 'Get Trial By trialDbId',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'arguments'      => array(
          3 => array(
            'name' => 'trialDbId',
            'type' => 'string',
            'description' => 'n/a',
            'required' => FALSE,
          ),
        ),
        'aggregatable' => TRUE,
      ),
      'variables'      => array(
        'title'          => 'Variable List',
        'description'    => 'List variables',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_variable_json',
        ),
        'aggregatable' => TRUE,
      ),
      'variables/{observationVariableDbId}' => array(
        'title'          => 'Variable Details',
        'description'    => 'Variable details by observationVariableDbId',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'    => BRAPI_USE_PERMISSION,
          'POST'   => BRAPI_UPDATE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_variable_json',
        ),
        'arguments'      => array(
          3 => array(
            'name' => 'observationVariableDbId',
            'type' => 'string',
            'description' => 'n/a',
            'required' => FALSE,
          ),
        ),
        'aggregatable' => TRUE,
      ),
      // 'variables/datatypes' => array(
      //   'title'          => 'Variable Data Types',
      //   'description'    => 'Variable data type list',
      //   'datatypes'      => array('json'),
      //   'methods'        => array(
      //     'GET'  => BRAPI_USE_PERMISSION,
      //     'POST' => BRAPI_USE_PERMISSION,
      //   ),
      //   'page callback'  => 'brapi_v1_external_call_json',
      //   'aggregatable' => TRUE,
      // ),
      'variables-search' => array(
        'title'          => 'Variable Search',
        'description'    => 'Search variables',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_variable_search_json',
        ),
        'aggregatable' => TRUE,
      ),
      'vendors/plates/{vendorPlateDbId}' => array(
        'title'          => 'Plate Details',
        'description'    => 'Get plate details by vendorPlateId',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'arguments'      => array(
          3 => array(
            'name' => 'vendorPlateDbId',
            'type' => 'string',
            'description' => 'n/a',
            'required' => FALSE,
          ),
        ),
        'aggregatable' => TRUE,
      ),
      'vendors/plates-search' => array(
        'title'          => 'Plate Search',
        'description'    => 'Search for plates',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'aggregatable' => TRUE,
      ),
      'vendors/specifications' => array(
        'title'          => 'Vendor Specifications',
        'description'    => 'Get vendor specifications',
        'datatypes'      => array('json'),
        'methods'        => array(
          'GET'  => BRAPI_USE_PERMISSION,
          'POST' => BRAPI_USE_PERMISSION,
        ),
        'page callback'  => NULL,
        'active version' => BRAPI_SERVICE_VERSION,
        'callback versions' => array(
          '1.1' => 'brapi_v1_external_call_json',
        ),
        'aggregatable' => TRUE,
      ),
    );

    foreach ($brapi_calls as $call => $call_settings) {
      if (empty($call_settings['page callback'])) {
        $active_version = $call_settings['active version'];
        $active_call = $call_settings['callback versions'][$active_version];
        $brapi_calls[$call]['page callback'] = $active_call;
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
 *      - $object: the corresponding Chado object loaded from database or an
 *        array of objects for the 'filter' operation;
 *      - $field_name: the name of the record field to work on;
 *      - $op: the operation to perform. Can be one of 'create', 'read',
 *         'update', 'delete' and 'filter'. If not provided, 'read' is assumed;
 *      - $value (not provided for 'read' $op and optional for 'delete' $op): the
 *        new field value (or value to delete in case of 'delete' $op) or the
 *        values to use for filtering.
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
          'germplasmPUI' => array(
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['persistent unique identifier'],
            ),
            'field_type' => 'string',
          ),
          'germplasmPUIDrupalURL' => function ($data_type, $stock, $field_name, $op, $values = NULL) {
            global $base_url;
            $germplasm_pui = '';
            switch ($op) {
              case NULL:
                $germplasm_pui = array('read' => TRUE);
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
          'femaleParentStockId' => array(
            'table' => 'stock_relationship',
            'column' => 'subject_id',
            'foreign_key' => 'object_id',
            'selector' => array(
              'type_id' => $cv_settings['FemaleParentOf'],
            ),
            'field_type' => 'int',
          ),
          'parent1DbId' => function ($data_type, $stock, $field_name, $op, $values = NULL) {
            $parent1_db_id = '';
            switch ($op) {
              case NULL:
                $parent1_db_id = array('read' => TRUE);
                break;

              case 'read':
                $parent_stock_id = brapi_get_field(
                  $data_type,
                  $stock,
                  'femaleParentStockId'
                );
                $parent = chado_generate_var(
                  'stock',
                  array('stock_id' => $parent_stock_id)
                );
                $parent1_db_id = ($parent ? $parent->uniquename : '');

              default:
            }
            return $parent1_db_id;
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
          'parent2DbId' => function ($data_type, $stock, $field_name, $op, $values = NULL) {
            $parent2_db_id = '';
            switch ($op) {
              case NULL:
                $parent2_db_id = array('read' => TRUE);
                break;

              case 'read':
                $parent_stock_id = brapi_get_field(
                  $data_type,
                  $stock,
                  'maleParentStockId'
                );
                $parent = chado_generate_var(
                  'stock',
                  array('stock_id' => $parent_stock_id)
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
                $pedigree = array('read' => TRUE);
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
              'type_id' => $cv_settings['GermplasmSynonym'],
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
          'instituteCodeStockCollection' => function ($data_type, $stock, $field_name, $op, $values = NULL) {
            $institute_code_stock_collection = NULL;
            switch ($op) {
              case NULL:
                $institute_code_stock_collection = array('read' => TRUE);
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
          // Note: the use of MCPD 'institute code' should be privileged
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
          'instituteNameStockCollection' => function ($data_type, $stock, $field_name, $op, $values = NULL) {
            $institute_name_stock_collection = NULL;
            switch ($op) {
              case NULL:
                $institute_name_stock_collection = array('read' => TRUE);
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
      'sample' => array(
        'table' => 'stock',
        'identifier' => 'stock_id',
        'accession' => 'uniquename',
        'fields' => array(
          'sampleDbId' => 'uniquename',
          'sourceStockId' => array(
            'table' => 'stock_relationship',
            'column' => 'object_id',
            'foreign_key' => 'subject_id',
            'selector' => array(
              'type_id' => $cv_settings['SampleOf'],
            ),
            'field_type' => 'int',
          ),
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
          'notes' => array(
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['Notes'],
            ),
            'field_type' => 'string',
          ),
          'experimentId' => array(
            'table' => 'nd_experiment_stock',
            'column' => 'nd_experiment_id',
            'selector' => array(),
            'field_type' => 'string',
            // 'return_array' => TRUE,
            // 'use_ranking' => FALSE,
          ),
        // "observationUnitDbId": "abc123",
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
          'plantDbId' => array(
            'table' => 'stockprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['PlantDbId'],
            ),
            'field_type' => 'string',
          ),
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
          'sampleType' => array(
            'table' => 'cvterm',
            'column' => 'name',
            'object_key' => 'type_id',
            'foreign_key' => 'cvterm_id',
            'selector' => array(),
            'field_type' => 'string',
          ),
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
          'tissueType' => array(
            'table' => 'stock_cvterm',
            'column' => 'cvterm_id',
            'selector' => array(
              'cvterm_id' => $cv_settings['TypeOfTissue'],
              'is_not' => FALSE,
            ),
            'field_type' => 'cvterm',
          ),
        ),
      ),
      'marker' => array(
        'table' => 'feature',
        'identifier' => 'feature_id',
        'accession' => 'uniquename',
        'fields' => array(
          'markerDbId' => 'uniquename',
          'defaultDisplayName' => 'name',
          'type' => array(
            'table' => 'cvterm',
            'foreign_key' => 'cvterm_id',
            'object_key' => 'type_id',
            'column' => 'name',
            'selector' => array(),
            'field_type' => 'string',
          ),
          'synonyms' => array(
            'table' => 'featureprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['MarkerSynonym'],
            ),
            'field_type' => 'string',
          ),
          'residues' => 'residues',
          'refAlt' =>  function ($data_type, $feature, $field_name, $op, $values = NULL) {
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
                // @todo: replace UPAC codes by corresponding list of nucleotides.
                $ref_alt = array($residues);
                break;

              default:
            }
            return $ref_alt;
          },
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
        ),
      ),
      'allelematrix' => array(
        // 'table' => 'xxx',
        // 'identifier' => 'xx_id',
        // 'accession' => 'uniquename',
        // 'fields' => array(
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
      ),
      'trait'        => array(
        // 'table' => 'xxx',
        // 'identifier' => 'xx_id',
        // 'accession' => 'uniquename',
        // 'fields' => array(
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
      ),
      'study'        => array(
        // 'table' => 'xxx',
        // 'identifier' => 'xx_id',
        // 'accession' => 'uniquename',
        // 'fields' => array(
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
      ),
      'attribute'    => array(
        // 'table' => 'xxx',
        // 'identifier' => 'xx_id',
        // 'accession' => 'uniquename',
        // 'fields' => array(
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
      ),
      'program'      => array(
        'table' => 'project',
        'identifier' => 'project_id',
        'accession' => 'project_id',
        'fields' => array(
          'programDbId' => 'project_id',
          'name' => 'name',
          'abbreviation' => 'name',
          'objective' => 'description',
          'contact_id' => array(
            'table' => 'project_contact',
            'column' => 'contact_id',
            'selector' => array(),
            'field_type' => 'int',
            'return_array' => TRUE,
            'use_ranking' => FALSE,
          ),
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
        // 'table' => 'xxx',
        // 'identifier' => 'xx_id',
        // 'accession' => 'uniquename',
        // 'fields' => array(
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
      ),
      'phenotype'    => array(
        // 'table' => 'xxx',
        // 'identifier' => 'xx_id',
        // 'accession' => 'uniquename',
        // 'fields' => array(
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
      ),
      'variable'     => array(
        'table' => 'cvterm',
        'identifier' => 'cvterm_id',
        'accession' => 'name',
        'fields' => array(
          'observationVariableDbId' => 'cvterm_id',
          'name' => 'name',
          'ontologyDbId' => function ($data_type, $cvterm, $field_name, $op, $values = NULL) use ($cv_settings) {
            $lead_contact = NULL;
            switch ($op) {
              case NULL:
                $lead_contact = array('read' => TRUE);
                break;

              case 'read':
                $sql_query = '
                  SELECT c.name
                  FROM {contact} c
                    JOIN {project_contact pc USING (contact_id)
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
        // 'table' => 'xxx',
        // 'identifier' => 'xx_id',
        // 'accession' => 'uniquename',
        // 'fields' => array(
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
      ),
      'observation'  => array(
        // 'table' => 'xxx',
        // 'identifier' => 'xx_id',
        // 'accession' => 'uniquename',
        // 'fields' => array(
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
      ),
      'location'     => array(
        'table' => 'nd_geolocation',
        'identifier' => 'nd_geolocation_id',
        'accession' => 'nd_geolocation_id',
        'fields' => array(
          'locationDbId' => 'nd_geolocation_id',
          'name' => 'description',
          'locationType' => array(
            'table' => 'nd_geolocationprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['LocationType'],
            ),
            'field_type' => 'string',
          ),
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
          'latitude' => 'latitude',
          'longitude' => 'longitude',
          'altitude' => 'altitude',
          'instituteName' => array(
            'table' => 'nd_geolocationprop',
            'column' => 'value',
            'selector' => array(
              'type_id' => $cv_settings['InstituteName'],
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
                else if (!is_array($geolocationprops)) {
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
                break;

              default:
            }

            return $info;
          },
        ),
      ),
    );

    // Alter default settings.
    $storage_options = variable_get(BRAPI_STORAGE_OPTIONS, array());

    // Germplasm fields...
    $germplasm_fields = &$brapi_data_mapping['germplasm']['fields'];

    // Set "germplasmPUI" settings.
    $use_specified_settings = array_key_exists(
      $storage_options['germplasm_pui'],
      $germplasm_fields
    );
    if ($use_specified_settings) {
      $germplasm_fields['germplasmPUI'] =
        &$germplasm_fields[$storage_options['germplasm_pui']];
    }

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
