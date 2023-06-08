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
 *
 * @see main BrAPI hook documentation for CALL_SIGNATURE format details.
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
 *
 * @see main BrAPI hook documentation for CALL_SIGNATURE format details.
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

/**
 * Alter BrAPI data before it is saved.
 *
 * @param array &$versions
 *   BrAPI definitions. See brapi_get_definition() for details.
 *   
 * @param string &$version_key
 *   Requested version as a key string. Key structure:
 *   version_key = version + '#' + subversion
 *   Note: any character that is neither a word character, a dot or a dash is
 *   removed from the key.
 *   ex.: $version_key = 'v2#2.0'
 */
function hook_brapi_definition_alter(
  array &$versions,
  string &$version_key,
) {
  // Adds a delete method to /lists/{listDbId}.
  if (empty($versions[$version_key]['calls']['/lists/{listDbId}']['definition']['delete'])) {
    $versions[$version_key]['calls']['/lists/{listDbId}']['definition']['delete'] = [
      "tags" => [
        "Lists",
      ],
      "summary" => "Removes a specific list",
      "description" => "Removes a specific list",
      "parameters" => [
        [
          "name" => "listDbId",
          "in" => "path",
          "description" => "The unique ID of this generic list",
          "required" => true,
          "style" => "simple",
          "explode" => false,
          "schema" => [
            "type" => "string",
          ],
        ],
        [
          "name" => "Authorization",
          "in" => "header",
          "description" => "HTTP HEADER - Token used for Authorization\n\n<strong> Bearer {token_string} </strong>",
          "required" => false,
          "style" => "simple",
          "explode" => false,
          "schema" => [
            "pattern" => "^Bearer .*$",
            "type" => "string",
          ],
          "example" => "Bearer XXXX",
        ],
      ],
      "responses" => [
        200 => [
          "description" => "OK",
          "content" => [
            "application/json" => [
              "schema" => [
                "$ref" => "#/components/schemas/ListsSingleResponse",
              ],
            ],
          ],
        ],
        400 => [
          "description" => "Bad Request",
          "content" => [
            "application/json" => [
              "schema" => [
                "type" => "string",
              ],
              "example" => "ERROR - 2018-10-08T18:15:11Z - Malformed JSON Request Object\n\nERROR - 2018-10-08T18:15:11Z - Invalid query parameter\n\nERROR - 2018-10-08T18:15:11Z - Required parameter is missing",
            ],
          ],
        ],
        401 => [
          "description" => "Unauthorized",
          "content" => [
            "application/json" => [
              "schema" => [
                "type" => "string",
              ],
              "example" => "ERROR - 2018-10-08T18:15:11Z - Missing or expired authorization token",
            ],
          ],
        ],
        403 => [
          "description" => "Forbidden",
          "content" => [
            "application/json" => [
              "schema" => [
                "type" => "string",
              ],
              "example" => "ERROR - 2018-10-08T18:15:11Z - User does not have permission to perform this action",
            ],
          ],
        ],
        404 => [
          "description" => "Not Found",
          "content" => [
            "application/json" => [
              "schema" => [
                "type" => "string",
              ],
              "example" => "ERROR - 2018-10-08T18:15:11Z - The requested object DbId is not found",
            ],
          ],
        ],
      ],
    ];
  }
}
