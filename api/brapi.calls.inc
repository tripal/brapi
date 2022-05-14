<?php

/**
 * @file
 * Implementation of Breeding API calls.
 *
 * Global call parameters:
 *
 * - pageSize (int)
 *   The size of the pages to be returned.
 *   Default is 1000.
 *   Example: 'pageSize=1000'.
 *
 * - page (int)
 *   Which result page is requested. Page index start from 1 (not 0).
 *   Example: 'page=10'.
 *
 * - pretty (bool)
 *   If set (true), outputs JSON in a nice human-readable format.
 *
 * - debug (bool)
 *   If set (true), outputs debug information.
 */

// Includes constants.
require_once 'brapi.const.inc';

/**
 * Wrapper for all BrAPI calls.
 *
 * Allows other module to alter call output or error messages. Modules willing
 * to alter call output should implement
 * HOOK_<brapi_call>_alter(&$data, &$context);
 * where <brapi_call> should be replaced by the basic call path with non-word
 * characters being replaced by underscores. Available call path can be found
 * in brapi.const.inc "brapi_get_calls()" function: keys of the returned array
 * are the actual call path.
 * For instance, in order to alter 'germplasm-search' call, the xbrapi module
 * should implement:
 * @code
 * function xbrapi_brapi_germplasm_search_alter($data, $context) {}
 * @endcode
 *
 * Error raised by BrAPI can be altered by implementing
 * HOOK_<brapi_call>_brapi_error_alter(&$message);
 *
 * Other errors can be altered by implementing
 * HOOK_<brapi_call>_error_alter(&$message);
 *
 * @see hook_brapi_CALL_FUNC_NAME_alter()
 * @see hook_brapi_CALL_FUNC_NAME_brapi_error_alter()
 * @see hook_brapi_CALL_FUNC_NAME_error_alter()
 */
function brapi_call_wrapper($call_name) {
  // Get call function.
  $call_data = brapi_get_calls()[$call_name];
  // By default, not implemented/not provided.
  $call_func = 'brapi_v1_unimplemented_call_json';
  if ($call_data['active version']) {
    // Check which version has been specified.
    if (isset($call_data['callback versions'][$call_data['active version']])) {
      // Using a valid version.
      $call_func = $call_data['callback versions'][$call_data['active version']];
    }
    elseif ('proxy' == $call_data['active version']) {
      // Using simple proxy call.
      $call_func = 'brapi_v1_external_call_json';
    }
  }

  $args = array_slice(func_get_args(), 1);
  // Replace slash and other non-word characters by '_' and lower-case string.
  $alter_name = strtolower(preg_replace('/\W+/', '_', $call_name));
  // Remove leading and trailing underscores.
  $alter_name = preg_replace('/^_+/', '', $alter_name);
  $alter_name = preg_replace('/_+$/', '', $alter_name);
  try {
    $debug = array();
    $debug[] = "Call name: $call_name";
    $debug[] = "Handling function: $call_func";
    $debug[] = "Alter hook: hook_brapi_{$alter_name}(_brapi_error|_error)_alter";
    list($data, $metadata, $debug_data) = call_user_func_array($call_func, $args);
    $debug_data = array_merge($debug, $debug_data);
    // Allow other modules to alter output.
    $context = array(
      'metadata' => &$metadata,
      'debug_data' => &$debug_data,
    );
    drupal_alter('brapi_' . $alter_name, $data, $context);
    $output = brapi_json_prepare($data, $metadata, $debug_data);
  }
  catch (BrapiException $be) {
    // Drupal alter.
    $output = $be->getJsonOutput();
    $output['debug'][] = "Alter hook: hook_brapi_{$alter_name}_brapi_error_alter";
    drupal_alter('brapi_' . $alter_name . '_brapi_error', $output);
  }
  catch (Exception $e) {
    // Drupal alter.
    $output = brapi_raise_error_json(
      500,
      $e->getMessage()
    );
    $output['debug'][] = "Alter hook: hook_brapi_{$alter_name}_error_alter";
    drupal_alter('brapi_' . $alter_name . '_error', $output);
  }

  return brapi_json_output($output);
}

/**
 * Generic object search call.
 *
 * Returns a list (possibliy empty) of object matching the user filters.
 *
 * Supported GET parameters:
 * - matchMethod string
 *     Whether to restrict the search to exact (case insensitive) or wildcard.
 *     '\' can be used to escape characters matches.
 *     Possible values are:
 *       'exact': exact character match (case sensitive);
 *       'case_insensitive': case insensitive;
 *       'wildcard': Wildcard uses both '*' or percent ('%') sign for any number
 *         of character and '?' for one character matching.
 *     Default is 'exact'.
 *     Example: 'matchMethod=wildcard'.
 *
 * - include string
 *     Whether to include synonyms in the output.
 *     Possible values are:
 *       'synonyms': also search in synonyms.
 *     Default: empty (does not include synonyms)
 *     Example: 'include=synonyms'.
 *
 * @return array
 *   an array of objects.
 *
 * @ingroup brapi
 */
function brapi_v1_object_search_json($object_type, $parameters) {

  // Init status and debug data.
  $status = array();
  $debug_data = array();
  $metadata = brapi_prepare_metadata();

  // Get JSON input if some.
  try {
    $user_data = brapi_get_user_json_input();
  }
  catch (Exception $e) {
    $user_data = array();
    $debug_data[] = $e->getMessage();
  }

  // Combine POST/GET call parameters.
  $filters = array_merge(
    $user_data,
    brapi_get_url_call_args()
  );
  $non_exact_match = FALSE;
  if (isset($filters['matchMethod'])
      && (strtolower($filters['matchMethod']) != 'exact')) {
    $non_exact_match = strtolower($filters['matchMethod']);
  }

  $cv_settings = brapi_get_cv_settings();
  $data_mapping = brapi_get_data_mapping();

  // Add pre-filtering.
  if (!empty($data_mapping[$object_type]['selector'])) {
    $selectors = $data_mapping[$object_type]['selector'];
  }
  else {
    $selectors = array();
  }

  // Prepare the list of post-filtering functions.
  $post_filters = array();
  // Hash of identifiers.
  $identifiers = array();
  $id_field = $data_mapping[$object_type]['identifier'];
  foreach ($filters as $field => $values) {
    $field_identifiers = array();
    // Check if the field is available.
    if (isset($data_mapping[$object_type]['fields'][$field])) {
      $field_fetcher = brapi_get_field_fetcher($data_mapping, $object_type, $field);
      // Generate an array of values.
      if (is_array($values)) {
        // Collapse all first.
        $values = implode(',', $values);
      }
      if (FALSE !== strpos($values, ',')) {
        // Explode multiple values.
        $values = array_map('trim', explode(',', $values));
      }
      else {
        // Single value.
        $values = array($values);
      }

      switch (gettype($field_fetcher)) {
        case 'string':
          // Field is in object table.
          $selectors[$field_fetcher] = $values;
          $debug_data[] = "Filter on field '$field' ($field_fetcher)"
            . " with value(s): "
            . implode(', ', $values);
          // Add to wildcard search list if needed.
          if ($non_exact_match) {
            brapi_non_exact_search(
              array(
                $field => array(
                  'table' => $data_mapping[$object_type]['table'],
                  'column' => $field_fetcher,
                  'match' => $non_exact_match,
                ),
              )
            );
            $debug_data[] = "Using $non_exact_match on field '$field' ($field_fetcher)";
          }
          break;

        case 'array':
          $debug_data[] = "Filter on linked field '$field' with value(s): "
            . implode(', ', $values);
          // Get available values from database mapping.
          if (isset($field_fetcher['foreign_key'])) {
            $foreign_key = $field_fetcher['foreign_key'];
          }
          else {
            $foreign_key = $id_field;
          }
          // Fix values according to data types.
          switch ($field_fetcher['field_type']) {
            case 'bool':
              $values = array_map(
                function ($v) {
                  return (bool) $v;
                },
                $values
              );
              break;

            case 'int':
              $values = array_map(
                function ($v) {
                  return (int) $v;
                },
                $values
              );
              break;

            case 'float':
              $values = array_map(
                function ($v) {
                  return (float) $v;
                },
                $values
              );
              break;

            case 'date':
              // Input format YYYY-MM-DD (ISO_8601)
              $new_date_values = array();
              foreach ($values as $value) {
                if (preg_match('/^\d\d\d\d-\d\d-\d\d$/', $value)) {
                  $new_date = brapi_encode_date($value);
                  $new_date_values[] = $new_date;
                  $debug_data[] = "Formatted date '$value' into '$new_date'.";
                }
                else {
                  $debug_data[] = "Invalid date format '$value' ignored.";
                }
              }
              $values = $new_date_values;
              break;

            case 'string':
              if ($non_exact_match) {
                brapi_non_exact_search(
                  array(
                    $field => array(
                      'table' => $field_fetcher['table'],
                      'column' => $field_fetcher['column'],
                      'match' => $non_exact_match,
                    ),
                  )
                );
                $debug_data[] = "Using $non_exact_match on field '$field' (" . $field_fetcher['column'] . ")";
              }
              break;

            case 'cvterm':
              // Get terms cvterm_id.
              $term_ids = array();
              // Loop on each value.
              foreach ($values as $value) {
                // Try to get term.
                $term = brapi_get_cvterm($value, FALSE);
                if ($term) {
                  $term_ids[$term->cvterm_id] = TRUE;
                }
                else {
                  $sanitized_value = urlencode($value);
                  $status[] = array(
                    'code' => 'warning',
                    'message' => t(
                      "Term '@value' not found for field '@field'!",
                      array('@value' => $sanitized_value, '@field' => $field)
                    ),
                  );
                }
              }
              $values = array_keys($term_ids);
              break;

            default:
              throw new BrapiException(
                400,
                t(
                  "Bad request! Invalid field '@field' configuration type '@field_type'!",
                  array(
                    '@field'      => $field,
                    '@field_type' => $field_fetcher['field_type'],
                  )
                )
              );
          }

          // Do not treat single values as arrays.
          if (1 == count($values)) {
            $values = array_pop($values);
          }

          $selector = array_merge(
            $field_fetcher['selector'],
            array($field_fetcher['column'] => $values)
          );

          // We capture output buffer for Tripal error messages.
          ob_start();
          $records = chado_select_record(
             $field_fetcher['table'],
             array($foreign_key),
             $selector
          );
          if ($tripal_message = ob_get_clean()) {
            $debug_data[] = $tripal_message;
          }

          if (empty($records)) {
            // No matching result for current field, stop here.
            return array(
              array(
                'result' => array('data' => array()),
              ),
              $metadata,
              $debug_data,
            );
          }
          else {
            // Force Tripal return value to be an array for easier management.
            if ($records && !is_array($records)) {
              $records = array($records);
            }
            foreach ($records as $record) {
              $rec_id_value = $record->$foreign_key;
              $field_identifiers[$rec_id_value] = TRUE;
            }
          }

          // Check for intersection.
          if (!isset($identifiers[$foreign_key])) {
            // Not set yet.
            $identifiers[$foreign_key] = $field_identifiers;
          }
          else {
            // Make sure we intersect objects matching other fields.
            $identifiers[$foreign_key] = array_intersect_key(
              $identifiers[$foreign_key],
              $field_identifiers
            );
            if (0 == count($identifiers)) {
              // No intersection between fields, stop here.
              return array(
                array(
                  'result' => array('data' => array()),
                ),
                $metadata,
                $debug_data,
              );
            }
          }
          break;

        case 'object':
          if (brapi_data_field_supports($object_type, $field, 'filter')) {
            $debug_data[] = "Filter on extra field '$field' with value(s): "
              . (is_array($values) ? implode(', ', $values) : $values);
            // Prepare post-filter.
            $post_filters[] = function ($objects) use (
              $field_fetcher,
              $object_type,
              $field,
              $values
            ) {
              return $field_fetcher($object_type, $objects, $field, 'filter', $values);
            };
          }
          else {
            // Not supported.
            $debug_data[] = "Field '$field' not supported for filtering.";
          }
          break;

        default:
          $debug_data[] = "Field '$field' not supported for filtering (unsupported type " . gettype($field_fetcher) . ")";
          // Ends type switch.
      }
      // Ends existing fields check.
    }
    elseif (!brapi_is_query_parameter($field)) {
      $sanitized_field = urlencode($field);
      $debug_data[] = "Field '$sanitized_field' not found for filtering";
      $status[] = array(
        'code' => 'warning',
        'message' => t(
          "unknown field '@field' ignored",
          array('@field' => $sanitized_field)
        ),
      );
    }
    // Ends field loop.
  }

  // Check if identifiers should also be filtered.
  if (count(array_keys($identifiers))) {
    foreach ($identifiers as $object_key => $key_values) {
      if (isset($selectors[$object_key])) {
        $selectors[$object_key] =
          array_intersect(
            array_keys($key_values),
            $selectors[$object_key]
          );
      }
      else {
        $selectors[$object_key] = array_keys($key_values);
      }
    }
  }

  $debug_data[] = "Filter: " . print_r($selectors, TRUE);

  $options = array(
    'order_by' => array($data_mapping[$object_type]['accession'] => 'ASC'),
  );

  // We capture output buffer for Tripal error messages.
  ob_start();
  try {
    // Setup pager data.
    $count_result = chado_select_record(
      $data_mapping[$object_type]['table'],
      array('count(1)'),
      $selectors
    );
    $total_count = $count_result[0]->count;
    $metadata = brapi_prepare_metadata($total_count);

    if (isset($metadata['pagination']['pageSize'])
        && $metadata['pagination']['pageSize']
        && isset($metadata['pagination']['currentPage'])) {
      // Use client page selection.
      $options['pager'] = array(
        'limit' => $metadata['pagination']['pageSize'],
        'element' => $metadata['pagination']['currentPage'],
      );
    }
    else {
      // Default paging.
      $options['pager'] = array(
        'limit' => BRAPI_DEFAULT_PAGE_SIZE,
        'element' => 0,
      );
    }

    // Force pager values as Tripal 2 pager system is not working properly.
    brapi_force_pager($options['pager']);

    // Support pager and sorting.
    $chado_objects = chado_generate_var(
      $data_mapping[$object_type]['table'],
      $selectors,
      $options
    );
  }
  catch (Exception $e) {
    $debug_data[] = $e->getMessage();
  }
  if ($tripal_message = ob_get_clean()) {
    $debug_data[] = $tripal_message;
  }

  if (!isset($chado_objects) || !$chado_objects) {
    $chado_objects = array();
  }
  elseif (!is_array($chado_objects)) {
    $chado_objects = array($chado_objects);
  }

  // Process post-filtering.
  foreach ($post_filters as $post_filter) {
    $chado_objects = $post_filter($chado_objects);
  }

  $debug_data[] = "Got " . count($chado_objects) . "result(s).";

  // Generate Chado object data.
  $object_list = array();
  foreach ($chado_objects as $chado_object) {
    $object_list[] = call_user_func(
      $parameters['get_object_details'],
      $chado_object
    );
  }

  if (is_array($metadata['status'])) {
    $metadata['status'] = array_merge($metadata['status'], $status);
  }
  else {
    $metadata['status'] = $status;
  }

  $data = array(
    'result' => array('data' => $object_list),
  );

  $agg_parameters = array(
    'GET' => brapi_get_url_call_args(),
    'POST' => $user_data,
  );
  brapi_aggregate_call($data, $metadata, $debug_data, $agg_parameters);

  return array(
    $data,
    $metadata,
    $debug_data,
  );
}

/**
 * About Breeding API call.
 *
 * BRAPI: About
 * URL: brapi/v1
 * Displays Breeding API version information.
 *
 * GET: Outputs BRAPI "about" message.
 *
 * @return array
 *   response JSON structure.
 *
 * @ingroup brapi
 */
function brapi_v1_root_10_json() {
  global $base_url;
  return array(
    array(
      'version' => BRAPI_SERVICE_VERSION,
      'provider' => variable_get('site_name', t('Unknown')),
      'description' => variable_get('site_slogan', ''),
      'contact' => array(
        'email' => variable_get('site_mail', ''),
        'www'   => $base_url . '/',
      ),
    ),
    array(),
    array(),
  );
}

/**
 * User authentication call.
 *
 * BRAPI: Authentication
 * URL: brapi/v1/token
 * Manages user login, logout and current user information.
 *
 * GET: returns an information hash about current user if one.
 * POST: tries to log in the given user with the given credential on local site.
 * DELETE: logs out current user.
 *
 * Supported POST parameters:
 * - grant_type (string)
 *   The grant type. Only allowed value is 'password', but it can be ignored.
 *   Example: 'grant_type': 'password'
 *
 * - username (string, required)
 *   The user login.
 *   Example: 'username': 'admin'
 *
 * - password (string)
 *   The user password.
 *   Example: 'password': 'mylittlesecret'
 *
 * - client_id (string)
 *   The client_id that correspond to the remote client name.
 *   Currently ignored.
 *   Example: 'client_id': 'admin'
 *
 * Example:
 * - GET:
 *   brapi/v1/token?pretty=1
 * - POST:
 *   brapi/v1/token
 * @code
 *   {
 *     'grant_type': 'password',
 *     'username': 'admin',
 *     'password': 'toto1234',
 *     'client_id': 'root'
 *   }
 * @endcode
 *
 * @return array
 *   a JSON structure with the following fields:
 *   - userDisplayName
 *   - access_token
 *   - expires_in
 *
 * @ingroup brapi
 */
function brapi_v1_authentication_json() {

  // Enforce HTTPS use.
  if (!$_SERVER['HTTPS']) {
    throw new BrapiException(
      426,
      '',
      array(
        'error' => t('You must use HTTPS protocol to login. Login through unsecure HTTP is forbidden.'),
      ),
      array(),
      TRUE
    );
  }

  global $user;

  // Init status and debug data.
  $authentication_data = array();
  $metadata = brapi_prepare_metadata(0);
  $debug_data = array();

  // Get expiration time.
  $maxlifetime = ini_get("session.gc_maxlifetime");

  // GET: get current user.
  if ($_SERVER['REQUEST_METHOD'] === 'GET') {
    // Return current credential.
    if (user_is_logged_in()) {
      // @todo for admin, list all users logged in (with brapi roles?).
      $authentication_data =
        array(
          'userDisplayName' => $user->name,
          'access_token' => session_name() . '=' . session_id(),
          'client_id' => $user->name,
          'expires_in' => $maxlifetime,
        );
    }
    return array(
      array('result' => $authentication_data),
      $metadata,
      $debug_data,
    );
  }
  // POST: login process.
  elseif ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Login...
    // Get user login and password.
    try {
      $user_data = brapi_get_user_json_input();
    }
    catch (Exception $e) {
      throw new BrapiException(
        400,
        $e->getMessage()
      );
    }

    // Local client name.
    $username   = empty($user_data['username']) ? FALSE : $user_data['username'];
    $password   = empty($user_data['password']) ? FALSE : $user_data['password'];
    $grant_type = empty($user_data['grant_type']) ? FALSE : $user_data['grant_type'];
    // Remote client name.
    $client_id  = empty($user_data['client_id']) ? FALSE : $user_data['client_id'];

    // BRAPI OAuth2 support only allows "password" grant type
    // and if not set, assume "password".
    if (!empty($grant_type) && ('password' != $grant_type)) {
      throw new BrapiException(
        501,
        '',
        array('error' => t('Only "password" grant type is supported.')),
        array(),
        TRUE
      );
    }

    // Stop if already logged in.
    if (user_is_logged_in()) {
      throw new BrapiException(
        400,
        '',
        array(
          'error' => t(
            'You are already logged in as "@username". Please logout first.',
            array('@username' => $user->name)
          ),
        ),
        array(),
        TRUE
      );
    }

    // Prepare authentication.
    $form_state = array(
      'values' => array(
        'name' => $username,
        'pass' => $password,
      ),
    );

    // Try authenticate.
    user_login_authenticate_validate(NULL, $form_state);

    if ($form_state['uid']) {
      user_login_submit(NULL, $form_state);
      // Got user authenticated, get session name and value.
      $authentication_data = array(session_name(), session_id());
    }
    else {
      // Not authenticated, issue an error.
      throw new BrapiException(
        400,
        '',
        array('error' => t('Authentication failed!')),
        array(),
        TRUE
      );
    }

    $response = array(
      'userDisplayName' => $user->name,
      'access_token'    => implode('=', $authentication_data),
      'client_id'       => $user->name,
      'expires_in'      => $maxlifetime,
    );

    return array($response, $metadata, $debug_data);

  }
  // DELETE: logout process.
  elseif ($_SERVER['REQUEST_METHOD'] === 'DELETE') {
    // @todo check if admin and a token has been provided in JSON data.
    // Logout if needed.
    if (user_is_logged_in()) {
      module_invoke_all('user_logout', $user);
      // Destroy the current session, and reset $user to the anonymous user.
      session_destroy();
      $metadata['status'][] =
        array(
          'code' => 200,
          'message' => t("User has been logged out successfully."),
        );
    }
    else {
      $metadata['status'][] =
        array(
          'code' => 200,
          'message' => t("Current user was not logged in."),
        );
    }
    return array(array('result' => array()), $metadata, $debug_data);
  }
  // Unsupported.
  else {
    throw new BrapiException(
      400,
      '',
      array('error' => t('Unsupported method!')),
      array(),
      TRUE
    );
  }
}

/**
 * Call list/search call for BrAPI v1.0-1.1.
 *
 * BRAPI: Call search
 * URL:  brapi/v1/calls
 * Displays the list of available calls.
 *
 * GET: search and returns the list of supported BrAPI calls.
 *
 * Supported GET parameters:
 * - call (string)
 *     The name of a call.
 */
function brapi_v1_call_10_json($selected_call_name = NULL) {

  // Init status and debug data.
  $call_data = array();
  $metadata = brapi_prepare_metadata();
  $debug_data = array();

  // Get call parameters.
  $args = brapi_get_url_call_args();
  $datatype_filter = isset($args['datatype']) ? $args['datatype'] : FALSE;

  static $aggregation_options;
  // Get call aggregation settings.
  if (!isset($aggregation_options)) {
    $aggregation_options = variable_get(BRAPI_AGGREGATION_OPTIONS, array());
  }

  // GET: get list of calls.
  if ($_SERVER['REQUEST_METHOD'] === 'GET') {
    foreach (brapi_get_calls() as $call_name => $call_settings) {
      // Check for unsupported calls.
      if (!$call_settings['active version']) {
        continue;
      }
      // Check proxied-only calls.
      $proxied = FALSE;
      if ('proxy' == $call_settings['active version']) {
        if (!isset($aggregation_options[$call_name])
            || !count($aggregation_options[$call_name])) {
          continue;
        }
        $proxied = TRUE;
        // @todo Check properties of remote calls?
      }

      // Check call name.
      $has_requested_name = TRUE;
      if ($selected_call_name) {
        $has_requested_name = ($selected_call_name == $call_name);
      }

      // Check call supported datatypes.
      $supported_datatypes = array();
      if (isset($call_settings['datatypes'])) {
        $supported_datatypes = $call_settings['datatypes'];
      }
      $has_requested_filters = TRUE;
      if ($datatype_filter) {
        $has_requested_filters = brapi_array_contains(
          $datatype_filter,
          $supported_datatypes
        );
      }

      if ($call_name && $has_requested_name && $has_requested_filters) {
        $call_data[] = array(
          'call' => $call_name,
          'datatypes' => $call_settings['datatypes'],
          'methods' => array_keys($call_settings['methods']),
        );
        // @todo Manage current user permission for methods (except for /token).
      }
    }
    $metadata = brapi_prepare_metadata(count($call_data));

    return array(
      array('result' => array('data' => $call_data)),
      $metadata,
      $debug_data,
    );
  }
  // Unsupported.
  else {
    throw new BrapiException(
      400,
      '',
      array('error' => t('Unsupported method!')),
      array(),
      TRUE
    );
  }
}

/**
 * Call list/search call for BrAPI v1.2.
 *
 * BRAPI: Call search
 * URL:  brapi/v1/calls
 * Displays the list of available calls.
 *
 * GET: search and returns the list of supported BrAPI calls.
 *
 * Supported GET parameters:
 * - call (string)
 *     The name of a call.
 */
function brapi_v1_call_12_json($selected_call_name = NULL) {

  // Init status and debug data.
  $call_data = array();
  $metadata = brapi_prepare_metadata();
  $debug_data = array();

  // Get call parameters.
  $args = brapi_get_url_call_args();
  $datatype_filter = isset($args['datatype']) ? $args['datatype'] : FALSE;

  static $aggregation_options;
  // Get call aggregation settings.
  if (!isset($aggregation_options)) {
    $aggregation_options = variable_get(BRAPI_AGGREGATION_OPTIONS, array());
  }

  // GET: get list of calls.
  if ($_SERVER['REQUEST_METHOD'] === 'GET') {
    foreach (brapi_get_calls() as $call_name => $call_settings) {
      // Check for unsupported calls.
      if (!$call_settings['active version']) {
        continue;
      }
      // Check proxied-only calls.
      $proxied = FALSE;
      if ('proxy' == $call_settings['active version']) {
        if (!isset($aggregation_options[$call_name])
            || !count($aggregation_options[$call_name])) {
          continue;
        }
        $proxied = TRUE;
        // @todo Check properties of remote calls?
      }

      // Check call name.
      $has_requested_name = TRUE;
      if ($selected_call_name) {
        $has_requested_name = ($selected_call_name == $call_name);
      }

      // Check call supported datatypes.
      $supported_datatypes = array();
      if (isset($call_settings['datatypes'])) {
        $supported_datatypes = $call_settings['datatypes'];
      }
      $has_requested_filters = TRUE;
      if ($datatype_filter) {
        $has_requested_filters = brapi_array_contains(
          $datatype_filter,
          $supported_datatypes
        );
      }

      if ($call_name && $has_requested_name && $has_requested_filters) {
        $call_data[] = array(
          'call' => $call_name,
          'datatypes' => $call_settings['datatypes'],
          'methods' => array_keys($call_settings['methods']),
          'versions' => array_keys($call_settings['callback versions']),
        );
        // @todo Manage current user permission for methods (except for /token).
      }
    }
    $metadata = brapi_prepare_metadata(count($call_data));

    return array(
      array('result' => array('data' => $call_data)),
      $metadata,
      $debug_data,
    );
  }
  // Unsupported.
  else {
    throw new BrapiException(
      400,
      '',
      array('error' => t('Unsupported method!')),
      array(),
      TRUE
    );
  }
}

/**
 * Call list/search call for BrAPI v1.3.
 *
 * BRAPI: Call search
 * URL:  brapi/v1/calls
 * Displays the list of available calls.
 *
 * GET: search and returns the list of supported BrAPI calls.
 *
 * Supported GET parameters:
 * - call (string)
 *     The name of a call.
 */
function brapi_v1_call_13_json($selected_call_name = NULL) {
  // Stub for BrAPI v1.3 delegates to v1.2.
  brapi_v1_call_12_json($selected_call_name);
}

/**
 * Germplasm search call.
 *
 * BRAPI: Germplasm Search
 * URL: brapi/v1/germplasm-search
 * Returns a list of germplasm.
 *
 * GET: search and returns name-matching accessions.
 *
 * Other service requests use the server's internal germplasmDbId. This service
 * returns germplasm records that provide the germplasmDbId. For the requested
 * name or synonym, returns an array (possibly empty) of germplasm records that
 * match the search criteria.
 * - If there is none, an empty array is returned.
 * - If there is one or more than one match, returns an array of all germplasm
 *   records that match the search criteria.
 *
 * Supported GET parameters:
 * - name string
 *     The name or synonym.
 *     Example: 'name=morex', 'name=mor%', 'name=mo*', 'name=more?'.
 *
 * - matchMethod string
 *     Whether to restrict the search to exact (case insensitive) or wildcard.
 *     '\' can be used to escape characters matches.
 *     Possible values are:
 *       'exact': exact character match (case sensitive);
 *       'case_insensitive': case insensitive;
 *       'wildcard': Wildcard uses both '*' or percent ('%') sign for any number
 *         of character and '?' for one character matching.
 *     Default is 'exact'.
 *     Example: 'matchMethod=wildcard'.
 *
 * - include string
 *     Whether to include synonyms in the output.
 *     Possible values are:
 *       'synonyms': also search in synonyms.
 *     Default: empty (does not include synonyms)
 *     Example: 'include=synonyms'.
 *
 * Example:
 * - GET
 *   brapi/v1/germplasm-search?name=ITC060?&matchMethod=wildcard&include=&pageSize=2&page=1
 *
 * @return array
 *   a JSON structure. See official BrAPI documentation.
 *
 * @see https://github.com/plantbreeding/API/blob/master/Specification/Germplasm/GermplasmSearchPOST.md
 *
 * @ingroup brapi
 */
function brapi_v1_germplasm_search_10_json() {
  $cv_settings = brapi_get_cv_settings();
  $parameters = array(
    'get_object_details' => 'brapi_get_germplasm_details_10',
  );
  return brapi_v1_object_search_json('germplasm', $parameters);
}

/**
 * Germplasm details call.
 *
 * BRAPI: Germplasm Details by DbId
 * URL: brapi/v1/germplasm/\<id\>
 * Displays details of a germplasm.
 *
 * GET: returns details of the given accession.
 *
 * Example:
 * - GET
 *   brapi/v1/germplasm/01BEL084609
 *
 * @return array
 *   a JSON structure. See official BrAPI documentation.
 *
 * @see https://github.com/plantbreeding/API/blob/master/Specification/Germplasm/GermplasmDetailsByGermplasmDbId.md
 *
 * @ingroup brapi
 */
function brapi_v1_germplasm_10_json($germplasm_id = NULL) {

  $actions = array(
    'create' => 'brapi_v1_create_germplasm_10_json',
    'read'   => 'brapi_v1_read_germplasm_10_json',
    'update' => 'brapi_v1_update_germplasm_10_json',
    'delete' => 'brapi_v1_delete_germplasm_10_json',
    'list'   => 'brapi_v1_germplasm_search_10_json',
  );

  return brapi_process_crud('germplasm/{germplasmDbId}', $actions, 'germplasm', $germplasm_id);
}

/**
 * Create a new stock (germplasm) entry.
 *
 * Insert a new germplasm in the stock table and creates required related
 * entries such as missing organism or properties and associated cv terms.
 *
 * @param array $user_data
 *   A hash containing the stock field values.
 *   Supported keys (*: mandatory fields, **: not BrAPI standard):
 *     - commonCropName*: common crop name;
 *     - species*: the Chado organism species;
 *     - genus*: the Chado organism genus;
 *     - defaultDisplayName*: the Chado stock name;
 *     - germplasmDbId*: the Chado stock uniquename/stock_id can also be
 *       specified through the $uniquename parameter which has higher priority;
 *     - abbreviation**: the Chado organism abbreviation;
 *     - description**: the Chado stock description;
 *     - type_id**: a Chado cvterm_id used for stock types.
 * @param string $uniquename
 *   The stock.uniquename value to use for the new stock.
 *
 * @return array
 *   a JSON structure containing the stock details.
 */
function brapi_v1_create_germplasm_10_json($user_data, $uniquename = NULL) {

  $storage_options = variable_get(BRAPI_STORAGE_OPTIONS, array());

  // Make sure we got all the requested data.
  $required_fields = array(
    'commonCropName', 'species', 'genus', 'defaultDisplayName', 'germplasmDbId',
  );
  $optional_fields = array('germplasmName', 'synonyms');

  foreach ($required_fields as $required_field) {
    if (empty($user_data[$required_field])) {
      throw new BrapiException(
        400,
        t("Missing germplasm @field!", array('@field' => $required_field))
      );
    }
  }

  // Check if a uniquename has been specified.
  if (!$uniquename) {
    $uniquename = $user_data['germplasmDbId'];
  }
  // Make sure we got a uniquename.
  if (!$uniquename) {
    throw new BrapiException(
      400,
      t("Missing germplasm germplasmDbId!")
    );
  }
  $cv_settings = brapi_get_cv_settings();

  // Check if organism exists.
  $organism_selector = array(
    'genus' => $user_data['genus'],
    'species' => $user_data['species'],
  );
  // We capture output buffer for Tripal error messages.
  ob_start();
  $organism = chado_generate_var('organism', $organism_selector);
  $tripal_message = ob_get_clean();
  if (is_array($organism)) {
    $organism = current($organism);
  }

  if (!$organism) {
    // Try to insert organism.
    if (isset($user_data['commonCropName'])) {
      $organism_selector['common_name'] = $user_data['commonCropName'];
    }

    if (isset($user_data['abbreviation'])) {
      $organism_selector['abbreviation'] = $user_data['abbreviation'];
    }

    // We capture output buffer for Tripal error messages.
    ob_start();
    $organism = chado_insert_record('organism', $organism_selector);
    $tripal_message = ob_get_clean();
    if (empty($organism) || !$organism['organism_id']) {
      throw new BrapiException(
        500,
        t(
          "Unable to insert new species!\n@tripal_message",
          array('@tripal_message' => $tripal_message)
        )
      );
    }
    $organism = (object) $organism;
    $organism_text = $organism->species . ' (' . $organism->organism_id . ')';
    watchdog(
      'brapi',
      'A new organism has been inserted: @organism',
      array('@organism' => $organism_text),
      WATCHDOG_DEBUG
    );
  }

  // Check if the user specified a stock description.
  $description = '';
  if (isset($user_data['description'])) {
    $description = $user_data['description'];
  }

  // Check if the user specified a type_id.
  if (isset($user_data['type_id'])) {
    $type_id = $user_data['type_id'];
  }
  else {
    $type_id = $cv_settings['Germplasm'];
  }
  // Make sure we got a type_id.
  if (!$type_id) {
    throw new BrapiException(
      400,
      t("Unable to insert new germplasm! Missing a type_id. You can either provide it in your JSON data (as \"'type_id': Chado cvterm_id,\") or in the default setting page of BrAPI.")
    );
  }

  $values = array(
    'organism_id' => $organism->organism_id,
    'name' => $user_data['defaultDisplayName'],
    'uniquename' => $uniquename,
    'description' => $description,
    'type_id' => $type_id,
    'is_obsolete' => 0,
  );
  // Check how BrAPI DbId are handled.
  if (isset($storage_options['dbid'])
      && ('chado_id' == $storage_options['dbid'])) {
    $values['stock_id'] = $uniquename;
    $values['uniquename'] = 'germplasm_' . $uniquename;
  }
  // We capture output buffer for Tripal error messages.
  ob_start();
  $stock = chado_insert_record('stock', $values);
  $tripal_message = ob_get_clean();
  if (empty($stock) || !$stock['stock_id']) {
    throw new BrapiException(
      500,
      t(
        "Unable to insert new germplasm!\n@tripal_message",
        array('@tripal_message' => $tripal_message)
      )
    );
  }
  // Turn stock into an object.
  $stock = (object) $stock;
  $stock_text = $stock->uniquename . ' (' . $stock->stock_id . ')';

  watchdog(
    'brapi',
    'A new stock has been inserted: @stock',
    array('@stock' => $stock_text),
    WATCHDOG_DEBUG
  );

  // @todo store other fields.
  foreach ($optional_fields as $optional_field) {
    if (isset($user_data[$optional_field])) {
      brapi_set_field(
        'germplasm',
        $stock,
        $optional_field,
        $user_data[$optional_field],
        FALSE
      );
    }
  }

  return brapi_v1_read_germplasm_10_json($stock);
}

/**
 * Returns a JSON array containing the details of a given germplasm.
 *
 * @param object $stock
 *   A Tripal Chado stock object.
 *
 * @return array
 *   a JSON array containing the stock details.
 */
function brapi_v1_read_germplasm_10_json($stock) {
  $metadata = brapi_prepare_metadata(1);
  $debug_data = array();
  $germplasm_data = brapi_get_germplasm_details_10($stock);
  $data = array('result' => $germplasm_data);
  brapi_aggregate_call($data, $metadata, $debug_data);

  return array(
    $data,
    $metadata,
    $debug_data,
  );
}

/**
 * Returns germplasm details in an associative array.
 *
 * @param object $stock
 *   A Tripal Chado stock object.
 *
 * @return array
 *   the stock details in an associative array.
 */
function brapi_get_germplasm_details_10($stock) {
  $germplasm_data = array();

  $fields = array(
    'germplasmDbId', 'defaultDisplayName', 'germplasmName', 'accessionNumber',
    'germplasmPUI', 'synonyms', 'commonCropName', 'instituteCode',
    'instituteName', 'biologicalStatusOfAccessionCode', 'countryOfOriginCode',
    'genus', 'species', 'speciesAuthority', 'subtaxa', 'subtaxaAuthority',
    'donors', 'acquisitionDate',
  );

  // Handle 'include'.
  $fields = brapi_merge_fields_to_include($fields);

  foreach ($fields as $field_name) {
    $germplasm_data[$field_name] =
      brapi_get_field('germplasm', $stock, $field_name);
  }
  return $germplasm_data;
}

/**
 * Updates a germplasm.
 *
 * @param array $user_data
 *   User input data.
 * @param object $stock
 *   A Tripal Chado stock object.
 *
 * @return array
 *   a JSON structure containing the stock details.
 */
function brapi_v1_update_germplasm_10_json($user_data, $stock) {

  $cv_settings = brapi_get_cv_settings();
  $stock_match = array('stock_id' => $stock->stock_id);
  $stock_data = array();

  // Species to update?
  $has_species = isset($user_data['species']);
  $has_genus = isset($user_data['genus']);
  if ($has_species || $has_genus) {
    // Change stock organism.
    // Check if organism exists.
    $organism_selector = array();
    if ($has_species || $has_genus) {
      $organism_selector['genus'] = $user_data['genus'];
      $organism_selector['species'] = $user_data['species'];
    }
    elseif ($has_species) {
      $organism_selector['genus'] = $stock->organism->genus;
      $organism_selector['species'] = $user_data['species'];
    }
    elseif ($has_genus) {
      $organism_selector['genus'] = $user_data['genus'];
      $organism_selector['species'] = $stock->organism->species;
    }

    // We capture output buffer for Tripal error messages.
    ob_start();
    $organism = chado_generate_var('organism', $organism_selector);
    $tripal_message = ob_get_clean();
    if (is_array($organism)) {
      $organism = current($organism);
    }

    if (empty($organism) || !$organism->organism_id) {
      throw new BrapiException(
        404,
        t("Unable to update germplasm: the given species was not found!")
      );
    }

    $stock_data['organism_id'] = $organism->organism_id;
  }

  // Check if the user specified a stock description.
  if (isset($user_data['description'])) {
    $stock_data['description'] = $user_data['description'];
  }

  // Check if the user specified a type_id.
  if (isset($user_data['type_id'])) {
    $stock_data['type_id'] = $user_data['type_id'];
  }

  // Check if the user specified a new name.
  if (isset($user_data['defaultDisplayName'])) {
    $stock_data['name'] = $user_data['defaultDisplayName'];
  }

  // We capture output buffer for Tripal error messages.
  ob_start();
  $updated =
    chado_update_record(
      'stock',
      $stock_match,
      $stock_data,
      array('return_record' => FALSE)
    );
  $tripal_message = ob_get_clean();
  if (!$updated) {
    throw new BrapiException(
      404,
      t(
        "Failed to update germplasm genus/species!\n@tripal_message",
        array('@tripal_message' => $tripal_message)
      )
    );
  }

  watchdog(
    'brapi',
    'Stock has been updated: @stock',
    array(
      '@stock' => $stock->uniquename . ' (' . $stock->stock_id . ')',
    ),
    WATCHDOG_DEBUG
  );

  // Update other fields.
  $updatable_fields = array('germplasmName', 'synonyms');

  foreach ($updatable_fields as $updatable_field) {
    if (isset($user_data[$updatable_field])) {
      brapi_set_field(
        'germplasm',
        $stock,
        $updatable_field,
        $user_data[$updatable_field],
        TRUE
      );
    }
  }

  return brapi_v1_read_germplasm_10_json($stock);
}

/**
 * Removes a germplasm.
 *
 * @param object $stock
 *   The Tripal Chado stock object to remove.
 *
 * @return array
 *   a JSON structure with and empty data array.
 */
function brapi_v1_delete_germplasm_10_json($stock) {
  $metadata = brapi_prepare_metadata(1);
  $debug_data = array();

  ob_start();
  if (!chado_delete_record('stock', array('stock_id' => $stock->stock_id))) {
    $tripal_message = ob_get_clean();
    throw new BrapiException(
      500,
      t(
        "Unable to remove germplasm!\n@tripal_message",
        array('@tripal_message' => $tripal_message)
      )
    );
  }
  ob_get_clean();
  $metadata['status'][] =
    array(
      'code' => 200,
      'message' => t("Successfully removed germplasm."),
    );

  return array(array(), $metadata, $debug_data);
}

/**
 * Germplasm pedigree call.
 *
 * BRAPI: Germplasm Pedigree
 * URL: brapi/v1/germplasm/\<id\>/pedigree
 * Displays pedigree of a germplasm.
 *
 * About pedigree string:
 * ----------------------
 *
 * The pedigree identifies the parents and crossing sequences used to produce
 * the cultivar.  The method used to illustrate pedigrees is a slightly modified
 * version of the system proposed by Purdy et al. in 1969 (see Crop Sci. 8:405-
 * 406).  Use of abbreviations has been minimized.  Crosses are symbolized by
 * combinations of slash marks ("/") with female and male parents listed to left
 * and right side, respectively.  Numbers indicate the order in which crosses
 * were made:
 *
 *                           /  = primary cross
 *
 *                          /2/ = secondary cross
 *
 *                          /3/ = tertiary cross
 *
 *                          /X/ = Xth level cross, etc.
 *
 * Higher numbers indicate more recent crosses in the sequence.  The most recent
 * or final cross used to create a cultivar is indicated by the highest number
 * within the pedigree.  For example, the pedigree of "Scout" hard red winter
 * wheat is:
 *
 *      -------------------------------------------------------------------
 *      Example 1:  Comparison of Purdy pedigree nomenclature to a tree
 *                  diagram of the pedigree of Scout hard red winter wheat.
 *
 *            Scout = Nebred /2/ Hope / Turkey Red /3/ Cheyenne / Ponca
 *
 *                                       OR
 *
 *                         Hope        Turkey Red
 *                           |______ / _____|
 *                                   |
 *                                   |
 *                Nebred            "?"
 *                   |_____ /2/ _____|
 *                           |              Cheyenne        Ponca
 *                           |                  |______ / ____|
 *                           |                          |
 *                           |                          |
 *                          "?"                        "?"
 *                           |___________ /3/ __________|
 *                                         |
 *                                         |
 *                                       Scout
 *      -------------------------------------------------------------------
 *
 * In narrative terms, an unidentified progeny of a primary cross between "Hope"
 * hard red spring wheat and "Turkey Red" hard red winter wheat was selected and
 * crossed to "Nebred" hard red winter wheat.  One of the progeny selected from
 * the "Nebred/2/Hope/Turkey Red" sequence of crosses was crossed to another
 * unidentified progeny derived by crossing "Cheyenne" and "Ponca" hard red
 * winter wheats.  The cultivar Scout was selected from progeny resulting from
 * the final or "/3/" cross.  Specific generations and selection techniques
 * involved are not indicated, but may be obtained from the referenced
 * literature.
 *
 * Single slash marks are also used where the parents are known, but the exact
 * sequence of a series of crosses is unknown.  Backcrossing sequences are
 * indicated by use of an asterisk ("*") preceded or followed by a number to
 * indicate the total number of crosses made with the recurrent parent (see
 * Examples 2 and 3).  Left and right parentheses are used to bracket both the
 * pedigree and designation of breeding lines contained within a cultivar's
 * pedigree (see Example 4).  Commas are used to separate breeding line
 * pedigrees from designations within the parentheses.
 *
 * @code
 *      -------------------------------------------------------------------
 *      Example 2:  Pedigree with three backcrosses of female recurrent parent
 *                  for TAM 107 hard red winter wheat.
 *
 *                            TAM 107 = TAM 105*4 / Amigo
 *
 *                                         OR
 *
 *                                                  TAM 105      Amigo
 *                                                     |____ / ____|
 *                                                           |
 *                                         TAM 105           |
 *         1st backcross>                     |____ *2 / ____|
 *                                                    |
 *                                  TAM 105           |
 *         2nd backcross>              |____ *3 / ____|
 *                                             |
 *                         TAM 105             |
 *         3rd backcross>     |_____ *4 / _____|
 *                                    |
 *                                    |
 *                                 TAM 107
 *
 *
 *      Example 3:  Pedigree with three backcrosses of male recurrent parent
 *                  for Blueboy II soft red winter wheat.
 *
 *                      Blueboy II = Agent / Tascosa /2/ 4*Blueboy
 *
 *                                          OR
 *
 *               Agent      Tascosa
 *                 |____ / ____|
 *                       |
 *                       |          Blueboy
 *                       |____ /2/ ____|
 *                              |
 *                              |            Blueboy
 *          1st backcross>      |____ /2/ 2* ____|
 *                                       |
 *                                       |            Blueboy
 *          2nd backcross>               |____ /2/ 3* ____|
 *                                                |
 *                                                |            Blueboy
 *          3rd backcross>                        |____ /2/ 4* ____|
 *                                                         |
 *                                                         |
 *                                                     Blueboy II
 *
 *
 *      Example 4:  Use of parentheses to delineate breeding line used in the
 *                  pedigree of Pitic 62 hard red spring wheat.
 *
 *            Pitic 62 = Yaktana 54 /2/ (Sel. 26-1c, Norin 10 / Brevor)
 *                                       ^
 *                                       Indicates Sel. 26-1c as the male
 *                                       parent of the highest order cross for
 *                                       Pitic 62, with its own pedigree of
 *                                       "Norin 10 / Brevor".
 *                                      OR
 *
 *                                   Norin 10          Brevor
 *                                      |________ / ______|
 *                                                |
 *                                                |
 *                   Yaktana 54              Sel. 26-1c
 *                       |__________ /2/ _________|
 *                                    |
 *                                    |
 *                                 Pitic 62
 *
 *      -------------------------------------------------------------------
 * @endcode
 *
 * Narratives providing more detailed information are used where necessary for
 * clarification.  Pedigrees of cultivars screened from another cultivar are
 * listed as "pure line selections".  Pedigrees of cultivars phenotypically
 * selected from mixtures or out-crosses in commercial fields are listed as
 * "farmer selections" with the original source material identified wherever
 * possible.
 *
 * GET: returns pedigree of the given accession.
 *
 * Example:
 * - GET
 *   brapi/v1/germplasm/01BEL084609
 *
 * @return array
 *   a JSON structure. See official BrAPI documentation.
 *
 * @see http://wheat.pw.usda.gov/ggpages/gopher/administration/Template%20for%20Germplasm%20records.html
 * @see https://github.com/plantbreeding/API/blob/master/Specification/Germplasm/GermplasmDetailsByGermplasmDbId.md
 *
 * @ingroup brapi
 */
function brapi_v1_germplasm_germplasmdbid_pedigree_10_json($germplasm_id = NULL) {

  $actions = array(
    'read'   => 'brapi_v1_read_germplasm_germplasmdbid_pedigree_10_json',
  );

  return brapi_process_crud('germplasm/{germplasmDbId}', $actions, 'germplasm', $germplasm_id);
}

/**
 * Germplasm pedigree call v1.1.
 *
 * BRAPI: Germplasm Pedigree
 * URL: brapi/v1/germplasm/\<id\>/pedigree
 * Displays pedigree of a germplasm.
 *
 * @see http://wheat.pw.usda.gov/ggpages/gopher/administration/Template%20for%20Germplasm%20records.html
 */
function brapi_v1_germplasm_germplasmdbid_pedigree_11_json($germplasm_id = NULL) {

  $actions = array(
    'read'   => 'brapi_v1_read_germplasm_germplasmdbid_pedigree_11_json',
  );

  return brapi_process_crud('germplasm/{germplasmDbId}', $actions, 'germplasm', $germplasm_id);
}

/**
 * Returns a JSON array containing the details of a given germplasm.
 *
 * @param object $stock
 *   A Tripal Chado stock object.
 *
 * @return array
 *   a JSON array containing the stock details.
 */
function brapi_v1_read_germplasm_germplasmdbid_pedigree_10_json($stock) {
  $metadata = brapi_prepare_metadata(1);
  $debug_data = array();
  $germplasm_data = array();

  $fields = array(
    'germplasmDbId',
    'defaultDisplayName',
    'parent1Id',
    'parent2Id',
    'pedigree',
  );
  foreach ($fields as $field_name) {
    $germplasm_data[$field_name] =
      brapi_get_field('germplasm', $stock, $field_name);
  }

  $data = array('result' => $germplasm_data);
  brapi_aggregate_call($data, $metadata, $debug_data);

  return array(
    $data,
    $metadata,
    $debug_data,
  );
}

/**
 * Returns a JSON array containing the details of a given germplasm.
 *
 * @param object $stock
 *   A Tripal Chado stock object.
 *
 * @return array
 *   a JSON array containing the stock details.
 */
function brapi_v1_read_germplasm_germplasmdbid_pedigree_11_json($stock) {
  $metadata = brapi_prepare_metadata(1);
  $debug_data = array();
  $germplasm_data = array();

  $fields = array(
    'germplasmDbId',
    'defaultDisplayName',
    'parent1DbId',
    'parent2DbId',
    'pedigree',
  );
  foreach ($fields as $field_name) {
    $germplasm_data[$field_name] =
      brapi_get_field('germplasm', $stock, $field_name);
  }

  $data = array('result' => $germplasm_data);
  brapi_aggregate_call($data, $metadata, $debug_data);

  return array(
    $data,
    $metadata,
    $debug_data,
  );
}

/**
 * Returns a JSON array containing the germplasm attribute categories.
 *
 * @return array
 *   a JSON array containing the germplasm attribute categories.
 */
function brapi_v1_attributes_11_json() {
  $germplasm_attributes = variable_get(BRAPI_GERMPLASM_ATTRIBUTES, array());
  $germplasm_categories = variable_get(BRAPI_GERMPLASM_ATTR_CATEGORIES, array());

  // Get call parameters.
  $args = brapi_get_url_call_args();
  $category_id = isset($args['attributeCategoryDbId']) ?
    $args['attributeCategoryDbId']
    : '0';
  // Note: attributeCategoryDbId is +1 than the real index in the category array
  // to avoid having a 0 ID. Therefore first category has attributeCategoryDbId
  // 1 but is $germplasm_categories[0].
  $debug_data = array();
  $attributes_data = array();
  foreach ($germplasm_attributes as $attribute_index => $attribute_settings) {
    // Filter categories if needed.
    if ($category_id && ($category_id != $attribute_settings['category'] + 1)) {
      continue;
    }
    list($term_filter, $filter_values) =
      brapi_v1_get_cvterm_filter($attribute_settings);

    // Only query if we got CVs or CV terms.
    if (count($filter_values[':cvterm_ids'])
        || count($filter_values[':parent_cvterm_ids'])
        || count($filter_values[':cv_ids'])) {
      // Get results.
      $sql_query = '
        SELECT
          cv.name AS "name",
          cv.definition AS "description",
          db.urlprefix || accession AS "uri",
          \'stock_cvterm\' AS "datatype",
          array_agg(DISTINCT
            CASE
              WHEN scvt.is_not = TRUE THEN \'NOT \'
              ELSE \'\'
            END
            || cvt.name
          ) AS "values"
        FROM {stock_cvterm} scvt
          JOIN {cvterm} cvt ON (cvt.cvterm_id = scvt.cvterm_id)
          JOIN {cv} cv ON (cv.cv_id = cvt.cv_id)
          JOIN {dbxref} dxr ON (dxr.dbxref_id = cvt.dbxref_id)
          JOIN {db} db ON (db.db_id = dxr.db_id)
        WHERE ('
        . $term_filter
        . ') GROUP BY cv.name, cv.definition, "uri", "datatype"
        UNION
        SELECT
          cvt.name AS "name",
          cvt.definition AS "description",
          db.urlprefix || accession AS "uri",
          \'stockprop\' AS "datatype",
          array_agg(DISTINCT sp.value) AS "values"
        FROM {stockprop} sp
          JOIN {cvterm} cvt ON (cvt.cvterm_id = sp.type_id)
          JOIN {dbxref} dxr ON (dxr.dbxref_id = cvt.dbxref_id)
          JOIN {db} db ON (db.db_id = dxr.db_id)
        WHERE ('
        . $term_filter
        . ') GROUP BY cvt.name, cvt.definition, "uri", "datatype"
      ;';

      $attribute_result = chado_query($sql_query, $filter_values);
      while ($attribute = $attribute_result->fetchAssoc()) {
        // Add some fields.
        $attribute['attributeCategoryDbId'] = (string) ($attribute_settings['category'] + 1);
        $attribute['code'] = $attribute_settings['name'];

        // Note: datatypeshould be like Categorical, Numeric, Boolean,...
        // Check if we must split values.
        if ($attribute['values']
            && (!is_array($attribute['values']))
            && ('{' == substr($attribute['values'], 0, 1))) {
          // Parse values.
          // Remove curly brackets (PostreSQL array).
          $attr_values = substr($attribute['values'], 1, -1);
          // We replace escaped quotes by the hex code so they won't interfere
          // in the regexp matching. All will be replaced by stripcslashes().
          $attr_values = preg_replace('/\\\\"/', '\\x22', $attr_values);
          // We match all double quoted and unquoted strings followed by comas.
          // Note: we append a coma to the value string to match last value
          // with the regexp.
          if (!preg_match_all(
            '/(?:(?:"([^"]+)")|([^",][^,]*)),/',
            $attr_values . ',',
            $attr_matches,
            PREG_SET_ORDER
          )) {
            // Regexp did not match any, keep the whole string.
            // Possible case: a value staring with but not finishing a double
            // quote.
            $attr_matches = array(array($attr_values, $attr_values, ''));
            $debug_data[] = 'Unable to parse "values": "' . $attribute['values'] . '"';
          }

          $attribute['values'] = array();
          // Loop on matches.
          for ($match_index = 0; $match_index < count($attr_matches); ++$match_index) {
            // Get the match either unquoted ([2]) or quoted ([1]).
            if (isset($attr_matches[$match_index][2])) {
              $attr_match = $attr_matches[$match_index][2];
            }
            else {
              $attr_match = $attr_matches[$match_index][1];
            }
            stripcslashes($attr_match);
            $attr_match = trim($attr_match);
            if ('' != $attr_match) {
              $attribute['values'][] = $attr_match;
            }
          }
        }
        $attributes_data[] = $attribute;
      }
    }
  }

  usort($attributes_data, function ($a, $b) {
    return strnatcmp($a['name'], $b['name']);
  });
  $metadata = brapi_prepare_metadata(count($attributes_data));

  if (isset($metadata['pagination']['pageSize'])
      && $metadata['pagination']['pageSize']
      && isset($metadata['pagination']['currentPage'])) {
    // Use client page selection.
    $page_size = $metadata['pagination']['pageSize'];
    $data_offset = $page_size * $metadata['pagination']['currentPage'];
    $attributes_data = array_splice(
      $attributes_data,
      $data_offset,
      $page_size
    );
  }
  else {
    // Default paging.
    $attributes_data = array_splice(
      $attributes_data,
      0,
      BRAPI_DEFAULT_PAGE_SIZE
    );
  }

  $data = array('result' => array('data' => $attributes_data));
  brapi_aggregate_call($data, $metadata, $debug_data);

  return array(
    $data,
    $metadata,
    $debug_data,
  );
}

/**
 * Returns a JSON array containing the germplasm attribute categories.
 *
 * @return array
 *   a JSON array containing the germplasm attribute categories.
 */
function brapi_v1_categories_11_json() {
  $germplasm_categories = variable_get(BRAPI_GERMPLASM_ATTR_CATEGORIES, array());

  $metadata = brapi_prepare_metadata(1);
  $debug_data = array();
  $categories_data = array();
  foreach ($germplasm_categories as $category_index => $category) {
    $categories_data[] = array(
      'attributeCategoryDbId' => (string) ($category_index + 1),
      'name' => $category,
    );
  }

  $data = array('result' => array('data' => $categories_data));
  brapi_aggregate_call($data, $metadata, $debug_data);

  return array(
    $data,
    $metadata,
    $debug_data,
  );
}

/**
 * Germplasm attributes call.
 *
 * BRAPI: Germplasm Attributes
 * URL: brapi/v1/germplasm/\<id\>/attributes
 * Displays attributes of a germplasm.
 *
 * GET: returns attributes of the given accession.
 *
 * Example:
 * - GET
 *   brapi/v1/germplasm/01BEL084609/attributes
 *
 * @return array
 *   a JSON structure. See official BrAPI documentation.
 *
 * @see https://github.com/plantbreeding/API/blob/master/Specification/GermplasmAttributes/GermplasmAttributeValuesByGermplasmDbId.md
 *
 * @ingroup brapi
 */
function brapi_v1_germplasm_germplasmdbid_attributes_10_json($germplasm_id = NULL) {

  $actions = array(
    'read'   => 'brapi_v1_read_germplasm_germplasmdbid_attributes_10_json',
  );

  return brapi_process_crud('germplasm/{germplasmDbId}', $actions, 'germplasm', $germplasm_id);
}

/**
 * Returns a JSON array containing the attributes of a given germplasm.
 *
 * @param object $stock
 *   A Tripal Chado stock object.
 *
 * @return array
 *   a JSON array containing the stock attributes (stock_cvterm+stockprop).
 */
function brapi_v1_read_germplasm_germplasmdbid_attributes_10_json($stock) {
  $germplasm_attributes = variable_get(BRAPI_GERMPLASM_ATTRIBUTES, array());
  $germplasm_categories = variable_get(BRAPI_GERMPLASM_ATTR_CATEGORIES, array());

  // Get call parameters.
  $args = brapi_get_url_call_args();
  $attribute_list = isset($args['attributeList']) ?
    explode(',', $args['attributeList'])
    : FALSE;
  // Note: $attribute_list is a list of attributeDbId. Each id correspond to a
  // 0-indexed $germplasm_attributes the following way:
  // $germplasm_attributes[$attribute_db_id - 1] (be aware of the " - 1" part).
  if ($attribute_list) {
    $new_germplasm_attributes = array();
    foreach ($attribute_list as $attribute_id) {
      $attribute_index = $attribute_id - 1;
      $new_germplasm_attributes[$attribute_index] =
        $germplasm_attributes[$attribute_index];
    }
    $germplasm_attributes = $new_germplasm_attributes;
  }

  $metadata = brapi_prepare_metadata(1);
  $debug_data = array();
  $germplasm_data = array();

  $fields = array(
    'germplasmDbId',
  );
  foreach ($fields as $field_name) {
    $germplasm_data[$field_name] =
      brapi_get_field('germplasm', $stock, $field_name);
  }
  // Attributes data.
  $germplasm_data['data'] = array();

  foreach ($germplasm_attributes as $attribute_index => $attribute_settings) {
    list($term_filter, $filter_values) =
      brapi_v1_get_cvterm_filter($attribute_settings);
    $filter_values[':stock_id'] = $stock->stock_id;
    $sql_query = '
      SELECT
        cv.name AS "attributeName",
        CASE
          WHEN scvt.is_not = TRUE THEN \'NOT \' || cvt.name
          ELSE cvt.name
        END AS "value"
      FROM {stock} s
        JOIN {stock_cvterm} scvt ON (s.stock_id = scvt.stock_id)
        JOIN {cvterm} cvt ON (cvt.cvterm_id = scvt.cvterm_id)
        JOIN {cv} cv ON (cv.cv_id = cvt.cv_id)
      WHERE
        s.stock_id = :stock_id AND ('
      . $term_filter
      . ')
      UNION
      SELECT
        cvt.name AS "name",
        sp.value AS "value"
      FROM {stock} s
        JOIN {stockprop} sp ON (s.stock_id = sp.stock_id)
        JOIN {cvterm} cvt ON (cvt.cvterm_id = sp.type_id)
      WHERE
        s.stock_id = :stock_id AND ('
      . $term_filter
      . ')
    ;';

    $attribute_result = chado_query($sql_query, $filter_values);
    while ($attribute_value = $attribute_result->fetchAssoc()) {
      $attribute_value['attributeDbId'] = (string) ($attribute_index + 1);
      $attribute_value['attributeCode'] = $attribute_settings['name'];
      $attribute_value['determinedDate'] = NULL;
      $germplasm_data['data'][] = $attribute_value;
    }
  }

  $data = array('result' => $germplasm_data);
  brapi_aggregate_call($data, $metadata, $debug_data);

  return array(
    $data,
    $metadata,
    $debug_data,
  );
}

/**
 * Crops call.
 *
 * BRAPI: Crops
 * URL: brapi/v1/crops
 * Displays list of supported crops.
 *
 * GET: returns suppoted crop list.
 *
 * Example:
 * - GET
 *   brapi/v1/crops
 *
 * @return array
 *   a JSON structure. See official BrAPI documentation.
 *
 * @see https://github.com/plantbreeding/API/blob/master/Specification/Crops/ListCrops.md
 *
 * @todo: add option to include all species, only stock species, just a static
 *   list, filter sub-species.
 *
 * @ingroup brapi
 */
function brapi_v1_crops_10_json() {
  $metadata = brapi_prepare_metadata(1);
  $debug_data = array();
  $crop_list = array();

  $storage_options = variable_get(BRAPI_STORAGE_OPTIONS, array());
  $naming_method = 'nameBySpecies';
  if (isset($storage_options['crop_naming_method'])) {
    $naming_method = $storage_options['crop_naming_method'];
  }

  if (isset($storage_options['crop_list'])
      && !empty($storage_options['crop_list'])) {
    $crop_list = explode(',', $storage_options['crop_list']);
  }
  else {
    if ($naming_method == 'nameByAbbreviation') {
      $sql_query = "
        SELECT abbreviation AS name, CONCAT_WS(' ', genus, species) AS altname, organism_id
        FROM {organism} o
        WHERE EXISTS
          (SELECT TRUE FROM {stock} s WHERE s.organism_id = o.organism_id LIMIT 1)
        ORDER BY abbreviation;";
    }
    else if ($naming_method == 'nameByCommonName') {
      $sql_query = "
        SELECT common_name AS name, CONCAT_WS(' ', genus, species) AS altname, organism_id
        FROM {organism} o
        WHERE EXISTS
          (SELECT TRUE FROM {stock} s WHERE s.organism_id = o.organism_id LIMIT 1)
        ORDER BY common_name;";
    }
    else if ($naming_method == 'nameByTaxon') {
      $sql_query = "
        SELECT NULL AS name, CONCAT_WS(' ', genus, species) AS altname, organism_id
        FROM {organism} o
        WHERE EXISTS
          (SELECT TRUE FROM {stock} s WHERE s.organism_id = o.organism_id LIMIT 1)
        ORDER BY genus, species;";
    }
    else {
      $sql_query = "
        SELECT species AS name, NULL AS altname
        FROM {organism} o
        WHERE EXISTS
          (SELECT TRUE FROM {stock} s WHERE s.organism_id = o.organism_id LIMIT 1)
        ORDER BY species;";
    }

    $crop_result = chado_query($sql_query);
    // Determine if we can lookup infraspecific nomenclature (chado>=1.3).
    $can_lookup_taxon = function_exists('chado_get_organism_scientific_name');
    while ($crop = $crop_result->fetchAssoc()) {
      $name = $crop['name'];
      $altname = $crop['altname'];
      // SQL ensures that if $name is missing for common_name or abbreviation, $altname will have a value.
      if (!$name) {
        if ($can_lookup_taxon) {
          $organism_id = $crop['organism_id'];
          $name = chado_get_organism_scientific_name(chado_get_organism(['organism_id' => $organism_id], []));
        }
        else {
          $name = $altname;
        }
      }
      $crop_list[] = $name;
    }
  }
  $data = array('result' => array('data' => $crop_list));
  brapi_aggregate_call($data, $metadata, $debug_data);

  return array(
    $data,
    $metadata,
    $debug_data,
  );
}

/**
 * Marker call.
 *
 * BRAPI: Markers
 * URL: brapi/v1/markers/<id>
 * Displays markers.
 *
 * GET: returns marker.
 *
 * Example:
 * - GET
 *   brapi/v1/markers
 *
 * @return array
 *   a JSON structure. See official BrAPI documentation.
 *
 * @see https://github.com/plantbreeding/API/blob/master/Specification/Markers/MarkerDetailsByMarkerDbId.md
 *
 * @ingroup brapi
 */
function brapi_v1_markers_11_json($marker_id = NULL) {
  $actions = array(
    'read'   => 'brapi_v1_read_markers_11_json',
    'list'   => 'brapi_v1_markers_search_11_json',
  );

  return brapi_process_crud('markers/{markerDbId}', $actions, 'marker', $marker_id);
}

/**
 * Returns marker details in an associative array.
 *
 * @param object $feature
 *   A Tripal Chado feature object.
 *
 * @return array
 *   the marker details in an associative array.
 */
function brapi_get_markers_details_10($feature) {
  $marker_data = array();

  $fields = array(
    'markerDbId',
    'defaultDisplayName',
    'type',
    'synonyms',
    'refAlt',
    'analysisMethods',
  );

  // Handle 'include'.
  $fields = brapi_merge_fields_to_include($fields);

  foreach ($fields as $field_name) {
    $marker_data[$field_name] =
      brapi_get_field('marker', $feature, $field_name);
  }
  return $marker_data;
}

/**
 * Returns a JSON array containing the details of a given marker.
 *
 * @param object $feature
 *   A Tripal Chado feature object.
 *
 * @return array
 *   a JSON array containing the feature details.
 */
function brapi_v1_read_markers_11_json($feature) {
  $metadata = brapi_prepare_metadata(1);
  $debug_data = array();
  $marker_data = array();

  // Check marker type.
  $cv_settings = brapi_get_cv_settings();
  $marker_type_ids = is_array($cv_settings['MarkerType']) ?
    $cv_settings['MarkerType']
    : array($cv_settings['MarkerType']);
  if (!in_array($feature->type_id->cvterm_id, $marker_type_ids)) {
    throw new BrapiException(404, t("Not a marker!"));
  }

  $marker_data = brapi_get_markers_details_10($feature);
  $data = array('result' => array('data' => $marker_data));
  brapi_aggregate_call($data, $metadata, $debug_data);

  return array(
    $data,
    $metadata,
    $debug_data,
  );
}

/**
 * Marker search call.
 *
 * BRAPI: Markers Search
 * URL: brapi/v1/markers-search
 * Returns a list of markers.
 *
 * GET: search and returns name-matching markers.
 *
 * @return array
 *   a JSON structure. See official BrAPI documentation.
 *
 * @see https://github.com/plantbreeding/API/blob/master/Specification/Markers/MarkerSearch_GET.md
 * @see https://github.com/plantbreeding/API/blob/master/Specification/Markers/MarkerSearch_POST.md
 *
 * @ingroup brapi
 */
function brapi_v1_markers_search_11_json() {
  $cv_settings = brapi_get_cv_settings();
  $parameters = array(
    'get_object_details' => 'brapi_get_markers_details_10',
  );

  return brapi_v1_object_search_json('marker', $parameters);
}

/**
 * Location call.
 *
 * BRAPI: Location
 * URL: brapi/v1/locations/<id>
 * Displays locations.
 *
 * GET: returns locations.
 *
 * Example:
 * - GET
 *   brapi/v1/locations
 *
 * @return array
 *   a JSON structure. See official BrAPI documentation.
 *
 * @see https://github.com/plantbreeding/API/blob/master/Specification/Locations/LocationDetails.md
 *
 * @ingroup brapi
 */
function brapi_v1_locations_11_json($location_id = NULL) {
  $actions = array(
    'read' => 'brapi_v1_read_locations_11_json',
    'list' => 'brapi_v1_locations_search_11_json',
  );

  return brapi_process_crud('locations/{locationDbId}', $actions, 'location', $location_id);
}

/**
 * Returns location details in an associative array.
 *
 * @param object $geolocation
 *   A Tripal Chado nd_geolocation object.
 *
 * @return array
 *   the location details in an associative array.
 */
function brapi_get_locations_details_11($geolocation) {
  $geolocation_data = array();

  $fields = array(
    'locationDbId',
    'name',
    'locationType',
    'abbreviation',
    'countryCode',
    'countryName',
    'latitude',
    'longitude',
    'altitude',
    'instituteName',
    'instituteAddress',
    'additionalInfo',
  );

  // Handle 'include'.
  $fields = brapi_merge_fields_to_include($fields);

  foreach ($fields as $field_name) {
    $geolocation_data[$field_name] =
      brapi_get_field('location', $geolocation, $field_name);
  }

  // Field name (a.k.a. 'description') must not be null.
  if (is_null($geolocation_data['name'])) {
    $geolocation_data['name'] = '';
  }

  // Field countryCode must not be null.
  if (is_null($geolocation_data['countryCode'])) {
    $geolocation_data['countryCode'] = '';
  }

  // Type cast needed for latitude, longitude and altitude.
  if (isset($geolocation_data['latitude']) && $geolocation_data['latitude']) {
    $geolocation_data['latitude'] = (float) $geolocation_data['latitude'];
  }
  if (isset($geolocation_data['longitude']) && $geolocation_data['longitude']) {
    $geolocation_data['longitude'] = (float) $geolocation_data['longitude'];
  }
  if (isset($geolocation_data['altitude']) && $geolocation_data['altitude']) {
    $geolocation_data['altitude'] = (float) $geolocation_data['altitude'];
  }

  return $geolocation_data;
}

/**
 * Returns a JSON array containing the details of a given location.
 *
 * @param object $geolocation
 *   A Tripal Chado nd_geolocation object.
 *
 * @return array
 *   a JSON array containing the geolocation details.
 */
function brapi_v1_read_locations_11_json($geolocation) {
  $metadata = brapi_prepare_metadata(1);
  $debug_data = array();
  $geolocation_data = array();

  $geolocation_data = brapi_get_locations_details_11($geolocation);
  $data = array('result' => $geolocation_data);
  brapi_aggregate_call($data, $metadata, $debug_data);

  return array(
    $data,
    $metadata,
    $debug_data,
  );
}

/**
 * Returns a JSON array containing the list of locations.
 *
 * @return array
 *   a JSON array containing the geolocations.
 */
function brapi_v1_locations_search_11_json() {
  $parameters = array(
    'get_object_details' => 'brapi_get_locations_details_11',
  );
  return brapi_v1_object_search_json('location', $parameters);
}

/**
 * Program call.
 *
 * BRAPI: Program
 * URL: brapi/v1/programs/<id>
 * Displays programs.
 *
 * GET: returns programs.
 *
 * Example:
 * - GET
 *   brapi/v1/programs
 *
 * @return array
 *   a JSON structure. See official BrAPI documentation.
 *
 * @see https://github.com/plantbreeding/API/blob/master/Specification/Programs/ListPrograms.md
 *
 * @ingroup brapi
 */
function brapi_v1_programs_11_json($program_id = NULL) {
  $actions = array(
    'read' => 'brapi_v1_read_programs_11_json',
    'list' => 'brapi_v1_programs_search_11_json',
  );

  return brapi_process_crud('programs/{programDbId}', $actions, 'program', $program_id);
}

/**
 * Returns program details in an associative array.
 *
 * @param object $project
 *   A Tripal Chado project object.
 *
 * @return array
 *   the program details in an associative array.
 */
function brapi_get_programs_details_11($project) {
  $project_data = array();

  $fields = array(
    'programDbId',
    'name',
    'abbreviation',
    'objective',
    'leadPerson',
  );

  // Handle 'include'.
  $fields = brapi_merge_fields_to_include($fields);

  foreach ($fields as $field_name) {
    $project_data[$field_name] =
      brapi_get_field('program', $project, $field_name);
  }
  // Lead person must not be null bu empty.
  if (is_null($project_data['leadPerson'])) {
    $project_data['leadPerson'] = '';
  }
  return $project_data;
}

/**
 * Returns a JSON array containing the details of a given program.
 *
 * @param object $project
 *   A Tripal Chado project object.
 *
 * @return array
 *   a JSON array containing the program details.
 */
function brapi_v1_read_programs_11_json($project) {
  $metadata = brapi_prepare_metadata(1);
  $debug_data = array();
  $project_data = array();

  $project_data = brapi_get_programs_details_11($project);
  $data = array('result' => array('data' => array($project_data)));
  brapi_aggregate_call($data, $metadata, $debug_data);

  return array(
    $data,
    $metadata,
    $debug_data,
  );
}

/**
 * Returns a JSON array containing the list of programs.
 *
 * @return array
 *   a JSON array containing the programs.
 */
function brapi_v1_programs_search_11_json() {
  $parameters = array(
    'get_object_details' => 'brapi_get_programs_details_11',
  );
  return brapi_v1_object_search_json('program', $parameters);
}

/**
 * Returns a JSON array containing a sample.
 *
 * @todo
 * This function needs to merge the sample-search GET code from
 *      'brapi_v1_samples_search_11_json'
 * with the sample creation PUT code of
 *       'brapi_v1_samples_11_json'.
 */
function brapi_v1_samples_13_json($sample_id = NULL) {
  throw new BrapiException(501, "Not implemented!");
}

/**
 * Sample search call.
 *
 * BRAPI: Sample Search
 * URL: brapi/v1/sample-search
 * Returns a list of sample.
 *
 * @return array
 *   a JSON structure. See official BrAPI documentation.
 *
 * @see https://github.com/plantbreeding/API/blob/master/Specification/Samples/SampleSearch_POST.md
 * @see https://github.com/plantbreeding/API/blob/master/Specification/Samples/SampleSearch_GET.md
 *
 * @ingroup brapi
 */
function brapi_v1_samples_search_11_json() {
  $cv_settings = brapi_get_cv_settings();
  $parameters = array(
    'get_object_details' => 'brapi_get_samples_details_11',
  );
  return brapi_v1_object_search_json('sample', $parameters);
}

/**
 * Sample details call.
 *
 * BRAPI: Sample Details by DbId
 * URL: brapi/v1/sample/\<id\>
 * Displays details of a sample.
 *
 * GET: returns details of the given accession.
 *
 * Example:
 * - GET
 *   brapi/v1/sample/01BEL084609
 *
 * @return array
 *   a JSON structure. See official BrAPI documentation.
 *
 * @see https://github.com/plantbreeding/API/blob/v1.3/Specification/Samples/Samples_GET.md
 *
 * @ingroup brapi
 */
function brapi_v1_samples_11_json($sample_id = NULL) {

  $actions = array(
    'create' => 'brapi_v1_create_samples_11_json',
    'read'   => 'brapi_v1_read_samples_11_json',
    'update' => 'brapi_v1_update_samples_11_json',
    'delete' => 'brapi_v1_delete_samples_11_json',
    'list'   => 'brapi_v1_samples_search_11_json',
  );

  return brapi_process_crud('samples/{sampleDbId}', $actions, 'sample', $sample_id);
}

/**
 * Create a new stock (sample) entry.
 *
 * Insert a new sample in the stock table and creates required related
 * entries such as missing organism or properties and associated cv terms.
 *
 * @param array $user_data
 *   A hash containing the stock field values.
 *   Supported keys (*: mandatory fields, **: not BrAPI standard):
 *     - commonCropName*: common crop name;
 *     - species*: the Chado organism species;
 *     - genus*: the Chado organism genus;
 *     - defaultDisplayName*: the Chado stock name;
 *     - sampleDbId*: the Chado stock uniquename. can also be specified
 *       through the $uniquename parameter which has higher priority;
 *     - abbreviation**: the Chado organism abbreviation;
 *     - description**: the Chado stock description;
 *     - type_id**: a Chado cvterm_id used for stock types.
 * @param string $uniquename
 *   The stock.uniquename value to use for the new stock.
 *
 * @return array
 *   a JSON structure containing the stock details.
 */
function brapi_v1_create_samples_11_json($user_data, $uniquename = NULL) {

  $storage_options = variable_get(BRAPI_STORAGE_OPTIONS, array());

  // Make sure we got all the requested data.
  $required_fields = array(
    'commonCropName', 'species', 'genus', 'defaultDisplayName', 'sampleDbId',
  );
  $optional_fields = array('sampleName', 'synonyms');

  foreach ($required_fields as $required_field) {
    if (empty($user_data[$required_field])) {
      throw new BrapiException(
        400,
        t("Missing sample @field!", array('@field' => $required_field))
      );
    }
  }

  // Check if a uniquename has been specified.
  if (!$uniquename) {
    $uniquename = $user_data['sampleDbId'];
  }
  // Make sure we got a uniquename.
  if (!$uniquename) {
    throw new BrapiException(
      400,
      t("Missing sample sampleDbId!")
    );
  }
  $cv_settings = brapi_get_cv_settings();

  // Check if organism exists.
  $organism_selector = array(
    'genus' => $user_data['genus'],
    'species' => $user_data['species'],
  );
  // We capture output buffer for Tripal error messages.
  ob_start();
  $organism = chado_generate_var('organism', $organism_selector);
  $tripal_message = ob_get_clean();
  if (is_array($organism)) {
    $organism = current($organism);
  }

  if (!$organism) {
    // Try to insert organism.
    if (isset($user_data['commonCropName'])) {
      $organism_selector['common_name'] = $user_data['commonCropName'];
    }

    if (isset($user_data['abbreviation'])) {
      $organism_selector['abbreviation'] = $user_data['abbreviation'];
    }

    // We capture output buffer for Tripal error messages.
    ob_start();
    $organism = chado_insert_record('organism', $organism_selector);
    $tripal_message = ob_get_clean();
    if (empty($organism) || !$organism['organism_id']) {
      throw new BrapiException(
        500,
        t(
          "Unable to insert new species!\n@tripal_message",
          array('@tripal_message' => $tripal_message)
        )
      );
    }
    $organism = (object) $organism;
    $organism_text = $organism->species . ' (' . $organism->organism_id . ')';
    watchdog(
      'brapi',
      'A new organism has been inserted: @organism',
      array('@organism' => $organism_text),
      WATCHDOG_DEBUG
    );
  }

  // Check if the user specified a stock description.
  $description = '';
  if (isset($user_data['description'])) {
    $description = $user_data['description'];
  }

  // Check if the user specified a type_id.
  if (isset($user_data['type_id'])) {
    $type_id = $user_data['type_id'];
  }
  else {
    $type_id = $cv_settings['Sample'];
  }
  // Make sure we got a type_id.
  if (!$type_id) {
    throw new BrapiException(
      400,
      t("Unable to insert new sample! Missing a type_id. You can either provide it in your JSON data (as \"'type_id': Chado cvterm_id,\") or in the default setting page of BrAPI.")
    );
  }

  $values = array(
    'organism_id' => $organism->organism_id,
    'name' => $user_data['defaultDisplayName'],
    'uniquename' => $uniquename,
    'description' => $description,
    'type_id' => $type_id,
    'is_obsolete' => 0,
  );
  // Check how BrAPI DbId are handled.
  if (isset($storage_options['dbid'])
      && ('chado_id' == $storage_options['dbid'])) {
    $values['stock_id'] = $uniquename;
    $values['uniquename'] = 'sample_' . $uniquename;
  }
  // We capture output buffer for Tripal error messages.
  ob_start();
  $stock = chado_insert_record('stock', $values);
  $tripal_message = ob_get_clean();
  if (empty($stock) || !$stock['stock_id']) {
    throw new BrapiException(
      500,
      t(
        "Unable to insert new sample!\n@tripal_message",
        array('@tripal_message' => $tripal_message)
      )
    );
  }
  // Turn stock into an object.
  $stock = (object) $stock;
  $stock_text = $stock->uniquename . ' (' . $stock->stock_id . ')';

  watchdog(
    'brapi',
    'A new stock has been inserted: @stock',
    array('@stock' => $stock_text),
    WATCHDOG_DEBUG
  );

  // @todo store other fields.
  foreach ($optional_fields as $optional_field) {
    if (isset($user_data[$optional_field])) {
      brapi_set_field(
        'sample',
        $stock,
        $optional_field,
        $user_data[$optional_field],
        FALSE
      );
    }
  }

  return brapi_v1_read_samples_11_json($stock);
}

/**
 * Returns a JSON array containing the details of a given sample.
 *
 * @param object $stock
 *   A Tripal Chado stock object.
 *
 * @return array
 *   a JSON array containing the stock details.
 */
function brapi_v1_read_samples_11_json($stock) {
  $metadata = brapi_prepare_metadata(1);
  $debug_data = array();
  $sample_data = brapi_get_samples_details_11($stock);
  $data = array('result' => $sample_data);
  brapi_aggregate_call($data, $metadata, $debug_data);

  return array(
    $data,
    $metadata,
    $debug_data,
  );
}

/**
 * Returns sample details in an associative array.
 *
 * @param object $stock
 *   A Tripal Chado stock object.
 *
 * @return array
 *   the stock details in an associative array.
 */
function brapi_get_samples_details_11($stock) {
  $sample_data = array();

  $fields = array(
    'germplasmDbId',
    'notes',
    'observationUnitDbId',
    'plantDbId',
    'plateDbId',
    'plateIndex',
    'plotDbId',
    'sampleDbId',
    'sampleTimestamp',
    'sampleType',
    'studyDbId',
    'takenBy',
    'tissueType',
  );

  // Handle 'include'.
  $fields = brapi_merge_fields_to_include($fields);

  foreach ($fields as $field_name) {
    $sample_data[$field_name] =
      brapi_get_field('sample', $stock, $field_name);
  }
  return $sample_data;
}

/**
 * Trait details call.
 *
 * BRAPI: Trait Details by DbId
 * URL: brapi/v1/traits/\<id\>
 * Displays details of a trait.
 *
 * GET: returns details of the given accession.
 *
 * Example:
 * - GET
 *   brapi/v1/trait/
 *
 * @return array
 *   a JSON structure. See official BrAPI documentation.
 *
 * @see https://github.com/plantbreeding/API/blob/master/Specification/Traits
 *
 * @ingroup brapi
 */
function brapi_v1_traits_11_json($trait = NULL) {

  $actions = array(
    'read'   => 'brapi_v1_traits_read_11_json',
    'list'   => 'brapi_v1_traits_search_11_json',
  );

  return brapi_process_crud('traits/{traitDbId}', $actions, 'trait', $trait);
}

/**
 * Scale details call.
 *
 * BRAPI: Scale Details by DbId
 * URL: brapi/v1/scales/\<id\>
 * Displays details of a scale.
 *
 * GET: returns details of the given scale.
 *
 * Example:
 * - GET
 *   brapi/v1/scales/
 *
 * @return array
 *   a JSON structure. See official BrAPI documentation.
 *
 * @see https://github.com/plantbreeding/API/blob/master/Specification/Scales
 *
 * @ingroup brapi
 */
function brapi_v1_scales_13_json($trait = NULL) {
  // Not implemented.
  throw new BrapiException(501, "Not implemented!");
}

/**
 * Method details call.
 *
 * BRAPI: Method Details by DbId
 * URL: brapi/v1/methods/\<id\>
 * Displays details of a method.
 *
 * GET: returns details of the given method.
 *
 * Example:
 * - GET
 *   brapi/v1/methods/
 *
 * @return array
 *   a JSON structure. See official BrAPI documentation.
 *
 * @see https://github.com/plantbreeding/API/blob/master/Specification/Methods
 *
 * @ingroup brapi
 */
function brapi_v1_methods_13_json($trait = NULL) {
  // Not implemented.
  throw new BrapiException(501, "Not implemented!");
}

/**
 * Returns a JSON array containing the details of a given trait.
 *
 * @param object $trait
 *   A Tripal Chado project object.
 *
 * @return array
 *   a JSON array containing the trait details.
 */
function brapi_v1_traits_read_11_json($trait) {
  $metadata = brapi_prepare_metadata(1);
  $debug_data = array();
  $trait_data = brapi_get_trait_details_11($trait);
  $data = array('result' => $trait_data);
  brapi_aggregate_call($data, $metadata, $debug_data);

  return array(
    $data,
    $metadata,
    $debug_data,
  );
}

/**
 * Trait search call.
 *
 * BRAPI: Trait Search
 * URL: brapi/v1/traits
 * Returns a list of traits.
 *
 * @return array
 *   a JSON structure. See official BrAPI documentation.
 *
 * @see https://github.com/plantbreeding/API/blob/master/Specification/Traits
 * @see https://github.com/plantbreeding/API/blob/master/Specification/Traits
 *
 * @ingroup brapi
 */
function brapi_v1_traits_search_11_json() {
  $cv_settings = brapi_get_cv_settings();
  $parameters = array(
    'get_object_details' => 'brapi_get_trait_details_11',
  );
  return brapi_v1_object_search_json('trait', $parameters);
}

/**
 * Returns Trait details in an associative array.
 *
 * @param object $trait
 *   A Tripal Chado cvterm object.
 *
 * @return array
 *   the trait details in an associative array.
 */
function brapi_get_trait_details_11($trait) {
  $trait_data = array();

  $fields = array(
    'description', 'name', 'observationVariables', 'traitDbId',
    'traitId', 'traitName', 'defaultValue',
  );

  // Handle 'include'.
  $fields = brapi_merge_fields_to_include($fields);

  foreach ($fields as $field_name) {
    $trait_data[$field_name] =
      brapi_get_field('trait', $trait, $field_name);
  }
  return $trait_data;
}

/**
 * Updates a sample.
 *
 * @param array $user_data
 *   User input data.
 * @param object $stock
 *   A Tripal Chado stock object.
 *
 * @return array
 *   a JSON structure containing the stock details.
 */
function brapi_v1_update_samples_11_json($user_data, $stock) {

  $cv_settings = brapi_get_cv_settings();
  $stock_match = array('stock_id' => $stock->stock_id);
  $stock_data = array();

  // Species to update?
  $has_species = isset($user_data['species']);
  $has_genus = isset($user_data['genus']);
  if ($has_species || $has_genus) {
    // Change stock organism.
    // Check if organism exists.
    $organism_selector = array();
    if ($has_species || $has_genus) {
      $organism_selector['genus'] = $user_data['genus'];
      $organism_selector['species'] = $user_data['species'];
    }
    elseif ($has_species) {
      $organism_selector['genus'] = $stock->organism->genus;
      $organism_selector['species'] = $user_data['species'];
    }
    elseif ($has_genus) {
      $organism_selector['genus'] = $user_data['genus'];
      $organism_selector['species'] = $stock->organism->species;
    }

    // We capture output buffer for Tripal error messages.
    ob_start();
    $organism = chado_generate_var('organism', $organism_selector);
    $tripal_message = ob_get_clean();
    if (is_array($organism)) {
      $organism = current($organism);
    }

    if (empty($organism) || !$organism->organism_id) {
      throw new BrapiException(
        404,
        t("Unable to update sample: the given species was not found!")
      );
    }

    $stock_data['organism_id'] = $organism->organism_id;
  }

  // Check if the user specified a stock description.
  if (isset($user_data['description'])) {
    $stock_data['description'] = $user_data['description'];
  }

  // Check if the user specified a type_id.
  if (isset($user_data['type_id'])) {
    $stock_data['type_id'] = $user_data['type_id'];
  }

  // Check if the user specified a new name.
  if (isset($user_data['defaultDisplayName'])) {
    $stock_data['name'] = $user_data['defaultDisplayName'];
  }

  // We capture output buffer for Tripal error messages.
  ob_start();
  $updated =
    chado_update_record(
      'stock',
      $stock_match,
      $stock_data,
      array('return_record' => FALSE)
    );
  $tripal_message = ob_get_clean();
  if (!$updated) {
    throw new BrapiException(
      404,
      t(
        "Failed to update sample genus/species!\n@tripal_message",
        array('@tripal_message' => $tripal_message)
      )
    );
  }

  watchdog(
    'brapi',
    'Stock has been updated: @stock',
    array(
      '@stock' => $stock->uniquename . ' (' . $stock->stock_id . ')',
    ),
    WATCHDOG_DEBUG
  );

  // Update other fields.
  $updatable_fields = array('sampleName', 'synonyms');

  foreach ($updatable_fields as $updatable_field) {
    if (isset($user_data[$updatable_field])) {
      brapi_set_field(
        'sample',
        $stock,
        $updatable_field,
        $user_data[$updatable_field],
        TRUE
      );
    }
  }

  return brapi_v1_read_samples_11_json($stock);
}

/**
 * Removes a sample.
 *
 * @param object $stock
 *   The Tripal Chado stock object to remove.
 *
 * @return array
 *   a JSON structure with and empty data array.
 */
function brapi_v1_delete_samples_11_json($stock) {
  $metadata = brapi_prepare_metadata(1);
  $debug_data = array();

  ob_start();
  if (!chado_delete_record('stock', array('stock_id' => $stock->stock_id))) {
    $tripal_message = ob_get_clean();
    throw new BrapiException(
      500,
      t(
        "Unable to remove sample!\n@tripal_message",
        array('@tripal_message' => $tripal_message)
      )
    );
  }
  ob_get_clean();
  $metadata['status'][] =
    array(
      'code' => 200,
      'message' => t("Successfully removed sample."),
    );

  return array(array(), $metadata, $debug_data);
}

/**
 * Observation variable call.
 *
 * BRAPI: variable
 * URL: brapi/v1/variables/<id>
 * Displays variables.
 *
 * GET: returns variables.
 *
 * Example:
 * - GET
 *   brapi/v1/variables
 *
 * @return array
 *   a JSON structure. See official BrAPI documentation.
 *
 * @see https://github.com/plantbreeding/API/blob/master/Specification/ObservationVariables/VariableDetails.md
 *
 * @ingroup brapi
 */
function brapi_v1_variables_11_json($variable_id = NULL) {
  $actions = array(
    'read' => 'brapi_v1_read_variables_11_json',
    'list' => 'brapi_v1_variables_search_11_json',
  );

  return brapi_process_crud('variables/{observationVariableDbId}', $actions, 'variable', $variable_id);
}

/**
 * Returns variable details in an associative array.
 *
 * @param object $cvterm
 *   A Tripal Chado cvterm object.
 *
 * @return array
 *   the variable details in an associative array.
 */
function brapi_get_variables_details_11($cvterm) {
  $cvterm_data = array();

  $fields = array(
    'observationVariableDbId',
    'name',
    'abbreviation',
    'objective',
    'leadPerson',
  );

  // Handle 'include'.
  $fields = brapi_merge_fields_to_include($fields);

  foreach ($fields as $field_name) {
    $cvterm_data[$field_name] =
      brapi_get_field('variable', $cvterm, $field_name);
  }
  return $cvterm_data;
}

/**
 * Returns a JSON array containing the details of a given variable.
 *
 * @param object $project
 *   A Tripal Chado project object.
 *
 * @return array
 *   a JSON array containing the variable details.
 */
function brapi_v1_read_variables_11_json($project) {
  $metadata = brapi_prepare_metadata(1);
  $debug_data = array();
  $project_data = array();

  $project_data = brapi_get_variables_details_11($project);
  $data = array('result' => array('data' => array($project_data)));
  brapi_aggregate_call($data, $metadata, $debug_data);

  return array(
    $data,
    $metadata,
    $debug_data,
  );
}

/**
 * Observation variable data types call.
 *
 * BRAPI: types
 * URL: brapi/v1/variables/datatypes
 * Displays variables.
 *
 * GET: returns variables data types.
 *
 * Example:
 * - GET
 *   brapi/v1/variables/datatypes
 *
 * @return array
 *   a JSON structure. See official BrAPI documentation.
 *
 * @see https://github.com/plantbreeding/API/blob/master/Specification/ObservationVariables/VariableDataTypeList.md
 *
 * @ingroup brapi
 */
function brapi_v1_variables_datatypes_11_json() {
  $metadata = brapi_prepare_metadata(1);
  $debug_data = array();
  $datatypes = array(
    'Code',
    'Duration',
    'Nominal',
    'Numerical',
    'Ordinal',
    'Text',
    'Date',
  );

  $data = array('result' => array('data' => $datatypes));
  brapi_aggregate_call($data, $metadata, $debug_data);

  return array(
    $data,
    $metadata,
    $debug_data,
  );
}

/**
 * Returns a JSON array containing the list of variable.
 *
 * @return array
 *   a JSON array containing the variables.
 */
function brapi_v1_variables_search_11_json() {
  $parameters = array(
    'get_object_details' => 'brapi_get_variables_details_11',
  );
  return brapi_v1_object_search_json('variable', $parameters);
}

/**
 * Generic callback for BrAPI call not supported by Drupal BrAPI module.
 *
 * Those calls can be processed by external BrAPI services.
 */
function brapi_v1_external_call_json() {

  // Init status and debug data.
  $status = array();
  $debug_data = array();
  $metadata = brapi_prepare_metadata();

  // Get JSON input if some.
  try {
    $user_data = brapi_get_user_json_input();
  }
  catch (Exception $e) {
    $user_data = array();
    $debug_data[] = $e->getMessage();
  }

  $parameters = array(
    'GET' => brapi_get_url_call_args(),
    'POST' => $user_data,
  );
  $data = array('result' => array('data' => array()));
  if (!brapi_aggregate_call($data, $metadata, $debug_data, $parameters)) {
    // Not implemented.
    throw new BrapiException(501, "Not implemented!");
  }

  return array(
    $data,
    $metadata,
    $debug_data,
  );
}

/**
 * Generic callback for BrAPI call not implemented by Drupal BrAPI module.
 */
function brapi_v1_unimplemented_call_json() {
  throw new BrapiException(501, "Not implemented!");
}
