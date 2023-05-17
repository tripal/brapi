<?php

use Drupal\brapi\Entity\BrapiDatatype;
use Drupal\Core\Entity\EntityStorageInterface;
use Drupal\Core\Routing\RouteMatchInterface;
/**
 * @file
 * Hooks specific to the Hooks Example module.
 */

/**
 * BrAPI hook documentation.
 *
 * If a 'response' key is added to the $context, it will be returned to Drupal.
 *
 * The CALL_SIGNATURE is composed by the method + the BrAPI major version +
 * the call name all in lower case with any consecutive non-word characters
 * replaced by underscores and with trailing underscores removed.
 * ex.: "get_v2_lists_listdbid" is the signature for the GET method of the call
 * /lists/{listDbId} of BrAPI v2.
 *
 */

/**
 * Replace any default BrAPI call implementation.
 *
 * If a 'response' key is added to the $context, it will be returned to Drupal.
 *
 * @param &$json_array
 *   Should be not set unless a previous hook implementation set it.
 *   It should return a full BrAPI result structure including metadata.
 *   
 * @param array &$context
 *   An array containing the following:
 *   - 'request': a \Symfony\Component\HttpFoundation\Request object;
 *   - 'config' : a \Drupal\Core\Config\ImmutableConfig object;
 *   - 'version': a version string, either 'v1' or 'v2';
 *   - 'call'   : the call name string. ex.: '/lists/{listDbId}'
 *   - 'method' : the lowercase method string ex.: 'get', 'post', 'put',...
 */
function hook_brapi_call_alter(&$json_array, array &$context) {
}

/**
 * Replaces the given BrAPI call implementation.
 *
 * If a 'response' key is added to the $context, it will be returned to Drupal.
 *
 * @param &$json_array
 *   Should be not set unless a previous hook implementation set it.
 *   It should return a full BrAPI result structure including metadata.
 *   
 * @param array &$context
 *   An array containing the following:
 *   - 'request': a \Symfony\Component\HttpFoundation\Request object;
 *   - 'config' : a \Drupal\Core\Config\ImmutableConfig object;
 *   - 'version': a version string, either 'v1' or 'v2';
 *   - 'call'   : the call name string. ex.: '/lists/{listDbId}'
 *   - 'method' : the lowercase method string ex.: 'get', 'post', 'put',...
 */
function hook_brapi_call_CALL_SIGNATURE_alter(&$json_array, array &$context) {
}

/**
 * Supports missing BrAPI call implementations.
 *
 * If a 'response' key is added to the $context, it will be returned to Drupal.
 *
 * @param &$json_array
 *   Should be not set unless a previous hook implementation set it.
 *   It should return a full BrAPI result structure including metadata.
 *   
 * @param array &$context
 *   An array containing the following:
 *   - 'request': a \Symfony\Component\HttpFoundation\Request object;
 *   - 'config' : a \Drupal\Core\Config\ImmutableConfig object;
 *   - 'version': a version string, either 'v1' or 'v2';
 *   - 'call'   : the call name string. ex.: '/lists/{listDbId}'
 *   - 'method' : the lowercase method string ex.: 'get', 'post', 'put',...
 */
function hook_brapi_unsupported_call_alter(&$json_array, array &$context) {
}

/**
 * Alter any call result array.
 *
 * If a 'response' key is added to the $context, it will be returned to Drupal.
 *
 * @param &$json_array
 *   Should contain a call result.
 *   
 * @param array &$context
 *   An array containing the following:
 *   - 'request': a \Symfony\Component\HttpFoundation\Request object;
 *   - 'config' : a \Drupal\Core\Config\ImmutableConfig object;
 *   - 'version': a version string, either 'v1' or 'v2';
 *   - 'call'   : the call name string. ex.: '/lists/{listDbId}'
 *   - 'method' : the lowercase method string ex.: 'get', 'post', 'put',...
 */
function hook_brapi_call_result_alter(&$json_array, array &$context) {
}

/**
 * Alter a given call result array.
 *
 * If a 'response' key is added to the $context, it will be returned to Drupal.
 *
 * @param &$json_array
 *   Should contain a call result.
 *   
 * @param array &$context
 *   An array containing the following:
 *   - 'request': a \Symfony\Component\HttpFoundation\Request object;
 *   - 'config' : a \Drupal\Core\Config\ImmutableConfig object;
 *   - 'version': a version string, either 'v1' or 'v2';
 *   - 'call'   : the call name string. ex.: '/lists/{listDbId}'
 *   - 'method' : the lowercase method string ex.: 'get', 'post', 'put',...
 */
function hook_brapi_call_CALL_SIGNATURE_result_alter(&$json_array, array &$context) {
  // This example changes ['result']['data'] from
  // [['value' => '01BEL084609'], ['value' => '01BEL084123']]
  // to ['01BEL084609', '01BEL084123'].
  if (!empty($json_array['result']['data'])) {
    $json_array['result']['data'] = array_map(
      function ($d) {
        return $d['value'] ?? '';
      },
      $json_array['result']['data']
    );
  }
}

/**
 * Alter BrAPI data before it is saved.
 *
 * @param array &$data
 *   Current BrAPI record data.
 *   
 * @param BrapiDatatype &$data_type
 *   BrAPI data type mapping instance.
 * 
 * @param EntityStorageInterface &$storage
 *   Storage.
 */
function hook_brapi_BRAPI_DATA_TYPE_save_alter(
  array &$data,
  BrapiDatatype &$data_type,
  EntityStorageInterface &$storage
) {
}
