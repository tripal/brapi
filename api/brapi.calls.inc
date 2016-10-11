<?php

/**
 * @file
 * Implementation of Breeding API calls.
 *
 * General call parameters:
 *
 * - pageSize (integer)
 *   The size of the pages to be returned.
 *   Default is 1000.
 *   Example: 'pageSize=1000'.
 *
 * - page (integer)
 *   Which result page is requested. Page index start from 1 (not 0).
 *   Example: 'page=10'.
 *
 * - pretty (boolean)
 *   If set (true), outputs JSON in a nice human-readable format.
 *
 */

/**
 * BRAPI: About
 *        brapi/v1
 *
 * GET: Outputs BRAPI "about" message.
 *
 * @return string
 *   a JSON string.
 *
 * @ingroup brapi
 */
function brapi_v1_about_json() {
  global $base_url;
  return brapi_output_json(
    array(
      'version' => '1.0',
      'provider' => variable_get('site_name'),
      'description' => variable_get('site_slogan'),
      'contact' => array(
        'email' => variable_get('site_mail'),
        'www'   => $base_url . '/',
      ),
    ),
    array()
  );
}

/**
 * BRAPI: Authentication
 *        brapi/v1/token
 *
 * GET: returns an information hash about current user if one.
 * POST: tries to log in the given user with the given credential on local site.
 * DELETE: logs out current user.
 *
 * Supported POST parameters:
 * - grant_type string
 *   The grant type. Only allowed value is 'password', but it can be ignored.
 *   Example: 'grant_type': 'password'
 *
 * - username string required
 *   The user login.
 *   Example: 'username': 'admin'
 *
 * - password string
 *   The user password.
 *   Example: 'password': 'mylittlesecret'
 *
 * - client_id string
 *    The client_id that correspond to the remote client name.
 *    Currently ignored.
 *    Example: 'client_id': 'admin'
 *
 * Example:
 * - GET:
 *   brapi/v1/token?pretty=1
 * - POST:
 *   brapi/v1/token
 *   @code
 *   {
 *     'grant_type': 'password',
 *     'username': 'admin',
 *     'password': 'toto1234',
 *     'client_id': 'root'
 *   }
 *   @endcode
 *
 * @return string
 *   a JSON string.
 *
 * @ingroup brapi
 */
function brapi_v1_authentication_json() {

  if (!$_SERVER['HTTPS']) {
    return brapi_raise_raw_error_json(
      426,
      array(
        'error' => t('You must use HTTPS protocol to login. Login through unsecure HTTP is forbidden.'),
      )
    );
  }

  global $user;

  // init status and debug data
  $authentication_data = array();
  $metadata = brapi_prepare_metadata(0);
  $debug_data = array();

  // get expiration time
  $maxlifetime = ini_get("session.gc_maxlifetime");

  // GET: get current user
  if ($_SERVER['REQUEST_METHOD'] === 'GET') {
    // return current credential
    if (user_is_logged_in()) {
      //+FIXME: for admin, list all users logged in (with brapi roles?)
      $authentication_data =
        array(
            'userDisplayName' => $user->name,
            'access_token' => session_name() . '=' . session_id(),
            'client_id' => $user->name,
            'expires_in' => $maxlifetime,
        );
    }
    return brapi_output_json(
      array('result' => $authentication_data),
      $metadata,
      $debug_data
    );
  }
    // POST: login process
  elseif ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // login...
    // get user login and password
    try {
      $user_data = brapi_get_user_json_input();
    }
    catch (Exception $e) {
      return brapi_raise_error_json(
        400,
        $e->getMessage()
      );
    }

    // local client name
    $username   = empty($user_data['username'])   ? FALSE : $user_data['username'];
    $password   = empty($user_data['password'])   ? FALSE : $user_data['password'];
    $grant_type = empty($user_data['grant_type']) ? FALSE : $user_data['grant_type'];
    // remote client name
    $client_id  = empty($user_data['client_id'])  ? FALSE : $user_data['client_id'];

    // BRAPI OAuth2 support only allows "password" grant type
    // and if not set, assume "password"
    if (!empty($grant_type) && ('password' != $grant_type)) {
      return brapi_raise_raw_error_json(
        501,
        array('error' => t('Only "password" grant type is supported.'))
      );
    }

    // stop if already logged in
    if (user_is_logged_in()) {
      return brapi_raise_raw_error_json(
        400,
        array(
          'error' => t(
            'You are already logged in as "@username". Please logout first.',
            array('@username' => $user->name,)
          ),
        )
      );
    }

    // prepare authentication
    $form_state = array(
      'values' => array(
        'name' => $username,
        'pass' => $password,
      ),
    );

    // try authenticate
    user_login_authenticate_validate(NULL, $form_state);

    //if ($user_id = user_authenticate($username, $password)) {
    if ($form_state['uid']) {
      user_login_submit(NULL, $form_state);
      // got user authenticated, get session name and value
      $authentication_data = array(session_name(), session_id());
    }
    else {
      // not authenticated, issue an error
      return brapi_raise_raw_error_json(
        400,
        array('error' => t('Authentication failed!'),)
      );
    }


    $response = array(
      'userDisplayName' => $user->name,
      'access_token'    => implode('=', $authentication_data),
      'client_id'       => $user->name,
      'expires_in'      => $maxlifetime,
    );

    return brapi_output_json($response, $metadata, $debug_data);

  }
    // DELETE: logout process
  elseif ($_SERVER['REQUEST_METHOD'] === 'DELETE') {
    //+FIXME: check if admin and a token has been provided in JSON data
    // logout if needed
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
    return brapi_output_json(array('result' => array()), $metadata, $debug_data);
  }
    // unsupported
  else {
    return brapi_raise_raw_error_json(
      400,
      array('error' => t('Unsupported method!'),)
    );
  }
}

/**
 * BRAPI: Germplasm Search by Name
 *        brapi/v1/germplasm
 *
 * GET: search and returns name-matching accessions.
 *
 * Other service requests use the server's internal germplasmDbId. This service
 * returns germplasm records that provide the germplasmDbId. For the requested
 * name or synonym, returns an array (possibyly empty) of germplasm records that
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
 *       'exact': exact character match (but case insensitive);
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
 *   brapi/v1/germplasm?name=ITC060?&matchMethod=wildcard&include=&pageSize=2&page=1
 *
 * @return string
 *   a JSON string with the following variable (some can be embeded):
 *
 * Variable            Datatype   Required  Description
 *------------------------------------------------------
 * metadata            object            Y  pagination, status
 * pagination          object            Y  pageSize, currentPage, totalCount,
 *                                          totalPages
 * status              list              Y  code, message
 * result              object            Y  data
 * data                array of objects  Y  Array (possibly empty) of germplasm
 *                                          records
 * germplasmDbId       string            Y  Internal db identifier
 * defaultDisplayName  string            Y  A string representing the germplasm
 *                                          that will be meaningful to the user
 * germplasmName       string               Name of the germplasm. It can be the
 *                                          prefered name and does not have to
 *                                          be unique
 * accessionNumber     string               This is the unique identifier for
 *                                          accessions within a genebank, and is
 *                                          assigned when a sample is entered
 *                                          into the genebank collection
 * germplasmPUI        string               Permanent identifier (e.g. URI, DOI,
 *                                          LSID)
 * pedigree            string               Cross name with optional selection
 *                                          history.
 * seedSource          string               Seed source
 * synonyms            array of string      List of other germplasm names
 *
 *
 * Example:
 *   {
 *     "metadata": {
 *       "status": [],
 *       "pagination": {
 *         "pageSize": "2",
 *         "currentPage": 1,
 *         "totalCount": 25,
 *         "totalPages": 13
 *       },
 *       "queryName": "ITC060?"
 *     },
 *     "result": {
 *       "data": [
 *         {
 *           "accessionNumber": "ITC0605",
 *           "germplasmDbId": "01BEL084605",
 *           "defaultDisplayName": "Japaraka no.2",
 *           "germplasmName": "Japaraka no.2",
 *           "germplasmPUI": "http://www.crop-diversity.org/mgis/accession/01BEL084605"
 *         },
 *         {
 *           "accessionNumber": "PNG064",
 *           "germplasmDbId": "01AUS04388",
 *           "defaultDisplayName": "Somani",
 *           "germplasmName": "Somani",
 *           "germplasmPUI": "http://www.crop-diversity.org/mgis/accession/01AUS04388"
 *         }
 *       ]
 *     }
 *   }
 *
 * @ingroup brapi
 */
function brapi_v1_germplasm_by_name_json() {

  // get call parameters
  $args = drupal_get_query_parameters();

  // init status and debug data
  $status = array();
  $debug_data = array();

  // SQL statement parts
  $join_clause = array();
  $where_clause = array();
  $query_arguments = array();
  $matching_operator = 'ILIKE';

  // get additional information to include in the output
  $include = array(); // old default: array('synonyms' => TRUE)
  if (isset($args['include'])) {
    $include = array_fill_keys(explode(',', $args['include']), TRUE);
  }

  // get name search pattern
  $search_pattern = isset($args['name'])?$args['name']:'';
  //+FIXME: list all entries if no search query provided

  // get match method
  $match_method = isset($args['matchMethod'])?$args['matchMethod']:'';
  // make sure the method is supported
  if ('wildcard' == $match_method) {
    // replace characters
    $search_pattern =
      str_replace(
        array('_', '?'),
        array('\_', '_'),
        $search_pattern
      );
    $search_pattern =
      preg_replace(
        '/(^|[^' . preg_quote(BRAPI_ESCAPE_CHARACTER) . '])\\*/',
        '\\1%',
        $search_pattern
      );
    $search_pattern = str_replace('\\*', '*', $search_pattern);
  }
  elseif ('insensitive' == $match_method) {
  }
  elseif ('exact' == $match_method) {
    // $matching_operator = 'LIKE';
  }
  elseif ($match_method) {
    // a method has been specified and its not the 'exact' match method, raise an error
    return brapi_raise_error_json(
      501,
      t(
        "Unknown match method '@match_method'",
        array('@match_method' => htmlentities($match_method),)
      )
    );
  }

  if (!empty($search_pattern)) {
    $join_clause[] = "
            INNER JOIN {stockprop} sp_match ON
              sp_match.stock_id = s.stock_id
            INNER JOIN {cvterm} cvt_match ON
              cvt_match.cvterm_id = sp_match.type_id
              AND cvt_match.name IN (
                'accession number',
                'other identification numbers associated with the accession'
              )
    ";
    $where_clause[] = "sp_match.value $matching_operator :accession_number";
    // if we want only to match accession number add:
    //   $where_clause[] = "cvt_match.name = 'accession number'";
    $query_arguments[':accession_number'] = $search_pattern;
  }

  // get trial identifier
  $trial_id = isset($args['trialID'])?$args['trialID']:'';
  if (!empty($trial_id)) {
    $join_clause[] = "
            INNER JOIN {stockprop} sp_trial ON
              sp_trial.stock_id = s.stock_id
            INNER JOIN {cvterm} cvt_trial ON
              cvt_trial.cvterm_id = sp_trial.type_id
              AND cvt_trial.name = 'trial identifier'
    ";
    $where_clause[] = "sp_trial.value $matching_operator :trial_id";
    $query_arguments[':trial_id'] = $trial_id;
    $debug_data[] = 'Trial identifier: ' . $trial_id;
  }

  // prepare count query
  $sql_count_query = "
    SELECT COUNT(DISTINCT s.stock_id) AS \"count\"
    FROM
      {cv} cv_mcpd,
      {stock} s
        " . implode("\n", $join_clause) . "
    WHERE
      s.is_obsolete = FALSE
      AND cv_mcpd.name = '" . BRAPI_MULTICROP_PASSPORT_ONTOLOGY_CV . "'
      AND cvt_match.cv_id  = cv_mcpd.cv_id
      " . (count($where_clause)? ' AND ' . implode(' AND ', $where_clause) : '')
  ;

  $stock_count_result = chado_query($sql_count_query, $query_arguments);
  $stock_count = floor($stock_count_result->fetchAssoc()['count']);

  $metadata = brapi_prepare_metadata($stock_count);
  $pager = '';
  if ($stock_count) {
    $pager = 'LIMIT ' . $metadata['pagination']['pageSize'] . "\n";

    if ($metadata['pagination']['currentPage'] > 0) {
      $offset =
        ($metadata['pagination']['currentPage'] - 1)
        * $metadata['pagination']['pageSize']
      ;
      $pager .= "OFFSET $offset\n";
    }
  }

  $germplasm_list = array();
  $sql_query = "
    SELECT DISTINCT s.stock_id,
           s.uniquename,
           s.name,
           (
             SELECT string_agg(cvt_an.name || '=' || sp_an.value, ',')
             FROM
              {stockprop} sp_an
                INNER JOIN cvterm cvt_an ON cvt_an.cvterm_id = sp_an.type_id
             WHERE
               cvt_an.cv_id  = cv_mcpd.cv_id
               AND cvt_an.name IN (
                 'accession number',
                 'other identification numbers associated with the accession'
               )
               AND sp_an.stock_id = s.stock_id
           ) AS \"identifiers\",
           (
             SELECT string_agg(sp_ana.value, ',')
             FROM
              {stockprop} sp_ana
                INNER JOIN cvterm cvt_ana ON cvt_ana.cvterm_id = sp_ana.type_id
             WHERE
               cvt_ana.cv_id  = cv_mcpd.cv_id
               AND cvt_ana.name = 'accession name'
               AND sp_ana.stock_id = s.stock_id
           ) AS \"accession_names\"
    FROM
      {cv} cv_mcpd,
      {stock} s
        " . implode("\n", $join_clause) . "
    WHERE
      s.is_obsolete = FALSE
      AND cv_mcpd.name = '" . BRAPI_MULTICROP_PASSPORT_ONTOLOGY_CV . "'
      AND cvt_match.cv_id  = cv_mcpd.cv_id
      " . (count($where_clause)? ' AND ' . implode(' AND ', $where_clause) : '')
  . " $pager
  ";
  $debug_data[] = "SQL Query: $sql_query";

  $stocks = chado_query($sql_query, $query_arguments);

  foreach ($stocks as $stock) {
    $accession_number = '';
    $synonyms = array($stock->uniquename);
    // split stockprop using ','
    foreach (explode(',', $stock->identifiers) as $identifier) {
      // split type and value using '='
      $stockprop = explode('=', $identifier);
      // separate "accession number" from "other identification numbers..."
      if ('accession number' == $stockprop[0]) {
        // accession number
        $accession_number = $stockprop[1];
      }
      else {
        // other identification numbers...
        $synonyms[] = $stockprop[1];
      }
    }

    $germplasm = array(
      'accessionNumber'    => $accession_number,
      'germplasmDbId'      => $stock->uniquename,      // required
      'defaultDisplayName' => $stock->name,            // required
      'germplasmName'      => $stock->accession_names,
      // 'breederCode'        => NULL,
      // 'synonyms'           => NULL,                 // never empty
      'germplasmPUI'       => NULL,                    // never empty
      // 'pedigree'           => NULL,
      // 'seedSource'         => NULL,
    );

    if (isset($include['synonyms']) && $include['synonyms']) {
      $index = array_search($accession_number, $synonyms);
      while (FALSE !== $index) {
        unset($synonyms[$index]);
        $index = array_search($accession_number, $synonyms);
      }
      $synonyms = array_values($synonyms);
      // otherIdentificationNumbersAssociatedWithTheAccession
      $germplasm['synonyms'] = array_unique($synonyms);
    }

    //if (isset($include['germplasmPUI']) && $include['germplasmPUI']) {
    $germplasm['germplasmPUI'] =
      tripal_stock_match_stocks_page($stock->uniquename);
    //}

    $germplasm_list[] = $germplasm;
  }

  // sort accessions by stock.uniquename
  $sort_germplasm = function($a, $b) {
    //return strcmp($a['synonyms'][0], $b['synonyms'][0]);
    return strcmp($a['germplasmDbId'], $b['germplasmDbId']);
  };
  usort($germplasm_list, $sort_germplasm);


  $metadata['status'] = array_merge($metadata['status'], $status);
  $metadata['queryName'] = isset($args['name']) ? $args['name'] : '';

  $debug_data[] = "pattern=$search_pattern"; // pattern debugging

  return brapi_output_json(
    array(
      'result' => array('data' => $germplasm_list),
    ),
    $metadata,
    $debug_data
  );
}


/**
 * BRAPI: Germplasm Details by DbId
 *        brapi/v1/germplasm/\<id\>
 *
 * GET: returns details of the given accession.
 *
 * Example:
 * - GET
 *   brapi/v1/germplasm/01BEL084609
 *
 * @return string
 *   a JSON string with the following variable (some can be embeded):
 *
 * Variable            Datatype   Required  Description
 *------------------------------------------------------
 * metadata            object            Y  pagination, status
 * pagination          object            Y  pageSize, currentPage, totalCount,
 *                                          totalPages
 * status              list              Y  code, message
 * result              object            Y  data
 * data                array of objects  Y  Array of 0 or 1 germplasm records
 * germplasmDbId       string            Y  Internal db identifier
 * defaultDisplayName  string            Y  A string representing the germplasm
 *                                          that will be meaningful to the user
 * germplasmName       string               Name of the germplasm. It can be the
 *                                          prefered name and does not have to
 *                                          be unique
 * accessionNumber     string               This is the unique identifier for
 *                                          accessions within a genebank, and is
 *                                          assigned when a sample is entered
 *                                          into the genebank collection
 * germplasmPUI        string               Permanent identifier (e.g. URI, DOI,
 *                                          LSID)
 * pedigree            string               Cross name with optional selection
 *                                          history.
 * seedSource          string               Seed source
 * synonyms            array of string      List of other germplasm names
 *
 *
 * Example:
 *   {
 *     "metadata": {
 *       "status": [],
 *       "pagination": {
 *         "pageSize": 1000,
 *         "currentPage": 1,
 *         "totalCount": 1,
 *         "totalPages": 1
 *       }
 *     },
 *     "result": {
 *       "data": [
 *         {
 *           "germplasmDbId": "01BEL084609",
 *           "defaultDisplayName": "Pahang",
 *           "germplasmName": "Pahang",
 *           "accessionNumber": "ITC0609",
 *           "germplasmPUI": "http://www.crop-diversity.org/mgis/accession/01BEL084609",
 *           "synonyms": [
 *             "01BEL084609",
 *             "ITC0609"
 *           ]
 *         }
 *       ]
 *     }
 *   }
 *
 * @ingroup brapi
 */
function brapi_v1_germplasm_json($germplasm_id, $view_mode = 'full', $langcode = NULL) {

  $actions = array(
    'create' => brapi_v1_create_germplasm_json,
    'read'   => brapi_v1_read_germplasm_json,
    'update' => brapi_v1_update_germplasm_json,
    'delete' => brapi_v1_delete_germplasm_json,
    // 'list'   => function (),
  );

  return brapi_process_crud('/germplasm', $actions, 'germplasm', $germplasm_id);
}

/**
 * Insert a new germplasm in the stock table and creates required related
 * entries such as missing organism or properties and associated cv terms.
 *
 * @param $user_data
 * a hash containing the stock field values.
 * Supported keys (*: mandatory fields, **: not BrAPI standard):
 *   -commonCropName*:
 *   -species*: the Chado organism species
 *   -genus*: the Chado organism genus
 *   -defaultDisplayName*: the Chado stock name
 *   -germplasmDbId*: the Chado stock uniquename. can also be specified through
 *     the $uniquename parameter which has higher priority
 *   -abbreviation**: the Chado organism abbreviation
 *   -description**: the Chado stock description
 *   -type_id**: a Chado cvterm_id used for stock types
 *
 *
 * @param $uniquename
 * The stock.uniquename value to use for the new stock.
 */
function brapi_v1_create_germplasm_json($user_data, $uniquename = NULL) {

  // make sure we got all the requested data
  $required_fields = array(
    'commonCropName', 'species', 'genus', 'defaultDisplayName', 'germplasmDbId',
  );
  $optional_fields = array('germplasmName', 'synonyms');

  foreach ($required_fields as $required_field) {
    if (empty($user_data[$required_field])) {
      return brapi_raise_error_json(
        400,
        t("Missing germplasm @field!", array('@field' => $required_field,))
      );
    }
  }

  // check if a uniquename has been specified
  if (!$uniquename) {
    $uniquename = $user_data['germplasmDbId'];
  }
  // make sure we got a uniquename
  if (!$uniquename) {
    return brapi_raise_error_json(
      400,
      t("Missing germplasm germplasmDbId!")
    );
  }
  $cv_settings = brapi_get_cv_settings();

  // check if organism exists
  $organism_selector = array(
    'genus' => $user_data['genus'],
    'species' => $user_data['species'],
  );
  $organism = chado_generate_var('organism', $organism_selector);
  if (is_array($organism)) {
    $organism = current($organism);
  }

  if (!$organism) {
    // try to insert organism
    if (array_key_exists('commonCropName', $user_data)) {
      $organism_selector['common_name'] = $user_data['commonCropName'];
    }

    if (array_key_exists('abbreviation', $user_data)) {
      $organism_selector['abbreviation'] = $user_data['abbreviation'];
    }

    // we capture output buffer for Tripal error messages
    ob_start();
    $organism = chado_insert_record('organism', $organism_selector);
    $tripal_message = ob_get_clean();
    if (empty($organism) || !$organism['organism_id']) {
      return brapi_raise_error_json(
        500,
        t(
          "Unable to insert new species!\n@tripal_message",
          array('@tripal_message' => $tripal_message)
        )
      );
    }
    $organism = (object)$organism;
    watchdog(
      'brapi',
      'A new organism has been inserted: @organism',
      array(
        '@organism' =>
          $organism->species
          . ' ('
          . $organism->organism_id
          . ')',
      ),
      WATCHDOG_DEBUG
    );
  }

  // check if the user specified a stock description
  $description = '';
  if (array_key_exists('description', $user_data)) {
    $description = $user_data['description'];
  }

  // check if the user specified a type_id
  if (array_key_exists('type_id', $user_data)) {
    $type_id = $user_data['type_id'];
  }
  else {
    $type_id = $cv_settings['Germplasm'];
  }
  // make sure we got a type_id
  if (!$type_id) {
    return brapi_raise_error_json(
      400,
      t("Unable to insert new germplasm! Missing a type_id. You can either provide it in your JSON data (as \"'type_id': Chado cvterm_id,\") or in the default setting page of BrAPI.")
    );
  }

  $values =  array(
    'organism_id' => $organism->organism_id,
    'name' => $user_data['defaultDisplayName'],
    'uniquename' => $uniquename,
    'description' => $description,
    'type_id' => $type_id,
    'is_obsolete' => 0,
  );
  // we capture output buffer for Tripal error messages
  ob_start();
  $stock = chado_insert_record('stock', $values);
  $tripal_message = ob_get_clean();
  if (empty($stock) || !$stock['stock_id']) {
    return brapi_raise_error_json(
      500,
      t(
        "Unable to insert new germplasm!\n@tripal_message",
        array('@tripal_message' => $tripal_message)
      )
    );
  }
  // turn stock into an object
  $stock = (object)$stock;

  watchdog(
    'brapi',
    'A new stock has been inserted: @stock',
    array(
      '@stock' => $stock->uniquename
        . ' ('
        . $stock->stock_id
        . ')',
    ),
    WATCHDOG_DEBUG
  );

  //+FIXME: store other fields
  foreach ($optional_fields as $optional_field) {
    if (array_key_exists($optional_field, $user_data)) {
      brapi_set_field(
        'germplasm',
        $stock,
        $optional_field,
        $user_data[$optional_field],
        FALSE
      );
    }
  }

  return brapi_v1_read_germplasm_json($stock);
}


function brapi_v1_read_germplasm_json($stock) {
  $metadata = brapi_prepare_metadata(1);
  $debug_data = array();
  $germplasm_data = array();

  $fields = array(
    'germplasmDbId', 'defaultDisplayName', 'germplasmName', 'accessionNumber',
    'germplasmPUI', 'synonyms',
  );
  foreach ($fields as $field_name) {
    $germplasm_data[$field_name] =
      brapi_get_field('germplasm', $stock, $field_name);
  }

  return brapi_output_json(
    array(
      'result' => array('data' => array($germplasm_data))
    ),
    $metadata,
    $debug_data
  );
}


function brapi_v1_update_germplasm_json($user_data, $stock) {

  $cv_settings = brapi_get_cv_settings();
  $stock_match = array('stock_id' => $stock->stock_id);
  $stock_data = array();

  // species update?
  $has_species = array_key_exists('species', $user_data);
  $has_genus = array_key_exists('genus', $user_data);
  if ($has_species || $has_genus) {
    // change stock organism
    // check if organism exists
    $organism_selector = array();
    if ($has_species || $has_genus) {
        $organism_selector['genus'] = $user_data['genus'];
        $organism_selector['species'] = $user_data['species'];
    }
    else if ($has_species) {
        //$stock = chado_expand_var($stock, 'table', 'organism');
        $organism_selector['genus'] = $stock->organism->genus;
        $organism_selector['species'] = $user_data['species'];
    }
    else if ($has_genus) {
        //$stock = chado_expand_var($stock, 'table', 'organism');
        $organism_selector['genus'] = $user_data['genus'];
        $organism_selector['species'] = $stock->organism->species;
    }

    $organism = chado_generate_var('organism', $organism_selector);
    if (is_array($organism)) {
      $organism = current($organism);
    }

    if (empty($organism) || !$organism->organism_id) {
      return brapi_raise_error_json(
        404,
        t("Unable to update germplasm: the given species was not found!")
      );
    }

    $stock_data['organism_id'] = $organism->organism_id;
  }

  // check if the user specified a stock description
  if (array_key_exists('description', $user_data)) {
    $stock_data['description'] = $user_data['description'];
  }

  // check if the user specified a type_id
  if (array_key_exists('type_id', $user_data)) {
    $stock_data['type_id'] = $user_data['type_id'];
  }

  // check if the user specified a new name
  if (array_key_exists('defaultDisplayName', $user_data)) {
    $stock_data['name'] = $user_data['defaultDisplayName'];
  }

  // we capture output buffer for Tripal error messages
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
    return brapi_raise_error_json(
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
      '@stock' => $stock->uniquename
        . ' ('
        . $stock->stock_id
        . ')',
    ),
    WATCHDOG_DEBUG
  );

  // update other fields
  $updatable_fields = array('germplasmName', 'synonyms');

  foreach ($updatable_fields as $updatable_field) {
    if (array_key_exists($updatable_field, $user_data)) {
      brapi_set_field(
        'germplasm',
        $stock,
        $updatable_field,
        $user_data[$updatable_field],
        TRUE
      );
    }
  }

  return brapi_v1_read_germplasm_json($stock);
}


function brapi_v1_delete_germplasm_json($stock) {
  $metadata = brapi_prepare_metadata(1);
  $debug_data = array();

  ob_start();
  if (!chado_delete_record('stock', array('stock_id' => $stock->stock_id))) {
    $tripal_message = ob_get_clean();
    return brapi_raise_error_json(
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
      'message' => t("Successfully removed germplasm.")
    );

  return brapi_output_json(array(), $metadata, $debug_data);
}


//+FIXME: remove the old implementation below once re-implemented
/**
 * BRAPI: Germplasm Multi Crop Passport Descriptor
 *        brapi/v1/germplasm/\<id\>/MCPD
 *
 * GET: returns several MCPD fields of the given accession.
 *
 * Example:
 * - GET
 *   brapi/v1/germplasm/01BEL084609/MCPD
 *
 * @return string
 *   a JSON string with the following variable (some can be embeded):
 *
 * Variable            Datatype   Required  Description
 *------------------------------------------------------
 * metadata            object            Y  pagination, status
 * pagination          object            Y  pageSize, currentPage, totalCount,
 *                                          totalPages
 * status              list              Y  code, message
 * result              object            Y  data
 * data                array of objects  Y  Array of 0 or 1 germplasm records
 * germplasmDbId       string            Y  Internal db identifier
 * defaultDisplayName  string            Y  A string representing the germplasm
 *                                          that will be meaningful to the user
 * germplasmName       string               Name of the germplasm. It can be the
 *                                          prefered name and does not have to
 *                                          be unique
 * accessionNumber     string               This is the unique identifier for
 *                                          accessions within a genebank, and is
 *                                          assigned when a sample is entered
 *                                          into the genebank collection
 * germplasmPUI        string               Permanent identifier (e.g. URI, DOI,
 *                                          LSID)
 * pedigree            string               Cross name with optional selection
 *                                          history.
 * seedSource          string               Seed source
 * synonyms            array of string      List of other germplasm names
 *
 * commonCropName      string               Common name fo the crop (e.g. wheat,
 *                                          rice, maize, cassava, banana)
 * instituteCode       string            Y  Institute that has bred the
 *                                          material. Note: The code may consist
 *                                          of the 3-letter ISO 3166 country
 *                                          code of the country where the
 *                                          institute is located plus a number
 *                                          (e.g. COL001) as recommended by FAO
 *                                          WIEWS
 * instituteName       string               Name of the institute (or person)
 *                                          that bred the material.
 * biologicalStatusOfAccessionCode  string  400) Breeding/research material 410)
 *                                          Breeder's line 411) Synthetic
 *                                          population 412) Hybrid 413) Founder
 *                                          stock/base population 414) Inbred
 *                                          line (parent of hybrid cultivar)
 *                                          415) Segregating population 416)
 *                                          Clonal selection 420) Genetic stock
 *                                          421) Mutant (e.g. induced/insertion
 *                                          mutants, tilling populations) 422)
 *                                          Cytogenetic stocks 423) Other
 *                                          genetic stocks (e.g. mapping
 *                                          populations)500) Advanced or
 *                                          improved cultivar (conventional
 *                                          breeding methods) 600) GMO (by
 *                                          genetic engineering) 999) Other
 * countryOfOriginCode  string              3-letter ISO 3166-1 code of the
 *                                          country in which the sample was bred
 *                                          or selected (breeding lines, GMOs,
 *                                          segregating populations, hybrids,
 *                                          modern cultivars, etc.).
 * typeOfGermplasmStorageCode  array of string  If germplasm is maintained under
 *                                          different types of storage, multiple
 *                                          choices are allowed. 10) Seed
 *                                          collection 11) Short term 12) Medium
 *                                          term 13) Long term 20) Field
 *                                          collection 30) In vitro collection
 *                                          40) Cryopreserved collection 50) DNA
 *                                          collection 99) Other (elaborate in
 *                                          REMARKS field)
 * genus               string               Genus name for taxon. Initial
 *                                          uppercase letter required.
 * species             string               Specific epithet portion of the
 *                                          scientific name in lowercase
 *                                          letters.
 * speciesAuthority    string
 * subtaxa             string               Subtaxon can be used to store any
 *                                          additional taxonomic identifier. The
 *                                          following abbreviations are allowed:
 *                                          'subsp.' (for subspecies); 'convar.'
 *                                          (for convariety); 'var.' (for
 *                                          variety); 'f.' (for form); 'Group'
 *                                          (for 'cultivar group').
 * subtaxaAuthority    string
 * donors              array of object      code of the donor institute and
 *                                          Identifier assigned to an accession
 *                                          by the donor, and permanent
 *                                          identifier.
 * acquisitionDate     string               Date on which the accession entered
 *                                          the collection where YYYY is the
 *                                          year, MM is the month and DD is the
 *                                          day. Missing data (MM or DD) should
 *                                          be indicated with hyphens or '00'
 *                                          [double zero].
 *
 * Example:
 *   {
 *     "metadata": {
 *       "status": [],
 *       "pagination": {
 *         "pageSize": 1000,
 *         "currentPage": 1,
 *         "totalCount": 1,
 *         "totalPages": 1
 *       }
 *     },
 *     "result": {
 *       "data": [
 *         {
 *           "germplasmDbId": "01BEL084609",
 *           "defaultDisplayName": "Pahang",
 *           "germplasmName": "Pahang",
 *           "accessionNumber": "ITC0609",
 *           "germplasmPUI": "http://www.crop-diversity.org/mgis/accession/01BEL084609",
 *           "synonyms": [
 *             "01BEL084609",
 *             "ITC0609"
 *           ],
 *           "commonCropName": "Pahang",
 *           "instituteCode": "01BEL084",
 *           "instituteName": "ITC",
 *           "biologicalStatusOfAccessionCode": "100 wild",
 *           "countryOfOriginCode": "UNK",
 *           "typeOfGermplasmStorageCode": "10 seed collection",
 *           "genus": "Musa",
 *           "species": "subsp. malaccensis",
 *           "donors": [
 *             {
 *               "donorAccessionNumber": "CIRAD II.04.01.001.002; (BB IR.296; CNPMF 122"
 *             }
 *           ],
 *           "acquisitionDate": "19890525"
 *         }
 *       ]
 *     }
 *   }
 *
 * @ingroup brapi
 * /
function brapi_v1_germplasm_mcpd_json($germplasm_id, $view_mode = 'full', $langcode = NULL) {
  $metadata = brapi_prepare_metadata(1);

  $debug_data = array();

  // get corresponding stock
  $germplasm_data = array();
  $selector = array(
    'uniquename' => $germplasm_id,
  );

  if ($stock = chado_generate_var('stock', $selector)) {
    // we got a matching stock

    // -germplasmDbId
    $germplasm_data['germplasmDbId'] = $stock->uniquename; // required

    // -defaultDisplayName
    $germplasm_data['defaultDisplayName'] = $stock->name;  // required

    // -germplasmName
    if (($accession_name = brapi_get_stock_property_values($stock->stock_id, 'accession name'))
        && count($accession_name)) {
      $germplasm_data['germplasmName'] = $accession_name[0];
    }

    // -accessionNumber
    if (($accession_number = brapi_get_stock_property_values($stock->stock_id, 'accession number'))
        && count($accession_number)) {
      $germplasm_data['accessionNumber'] = $accession_number[0];
    }

    // -germplasmPUI
    $germplasm_data['germplasmPUI']  = tripal_stock_match_stocks_page($stock->uniquename);

    // -pedigree
    // $germplasm_data['pedigree'] = NULL;

    // -seedSource
    // $germplasm_data['seedSource'] = NULL;

    // -synonyms
    $germplasm_data['synonyms'] = array($stock->uniquename);
    if (($synonyms = brapi_get_stock_property_values($stock->stock_id, 'other identification numbers associated with the accession'))
        && count($synonyms)) {
      $germplasm_data['synonyms'] = array_merge($germplasm_data['synonyms'], $synonyms);
    }

    // -commonCropName
    if (($common_crop_name = brapi_get_stock_property_values($stock->stock_id, 'common crop name', 'mgis_accession')) //+FIXME: CV
        && count($common_crop_name)) {
      $germplasm_data['commonCropName'] = $common_crop_name[0];
    }
    else {
      $germplasm_data['commonCropName'] = $stock->name;
    }

    // -instituteCode
    if (($institute_code = brapi_get_stock_property_values($stock->stock_id, 'institute code'))
        && count($institute_code)) {
      $germplasm_data['instituteCode'] = $institute_code[0];
    }
    else {
      $collection_options = array('return_array' => 1);
      $stock = chado_expand_var($stock, 'table', 'stockcollection_stock', $collection_options);
      $collections = $stock->stockcollection_stock;
      if (0 < count($collections)) {
        $collection = $collections[0]->stockcollection_id;
        $germplasm_data['instituteCode'] = $collection->uniquename;
      }
    }
    //+FIXME: get from stocktprop?

    // -instituteName
    if (($institute_name = brapi_get_stock_property_values($stock->stock_id, 'institute name'))
        && count($institute_name)) {
      $germplasm_data['instituteName'] = $institute_name[0];
    }
    else {
      $collection_options = array('return_array' => 1);
      $stock = chado_expand_var($stock, 'table', 'stockcollection_stock', $collection_options);
      $collections = $stock->stockcollection_stock;
      if (0 < count($collections)) {
        $collection = $collections[0]->stockcollection_id;
        $germplasm_data['instituteName'] = $collection->name;
      }
    }

    // -biologicalStatusOfAccessionCode
    if (($biological_status = brapi_get_stock_property_values($stock->stock_id, 'BiologicalStatusOfAccessionCode'))
        && count($biological_status)) {
      $germplasm_data['biologicalStatusOfAccessionCode'] = $biological_status[0];
    }
    elseif (($biological_status = brapi_get_stock_cvterm_values($stock->stock_id, array('parent_term' => 'BiologicalStatusOfAccessionCode', 'cv' => BRAPI_MULTICROP_PASSPORT_ONTOLOGY_CV)))
            && count($biological_status)) {
      $germplasm_data['biologicalStatusOfAccessionCode'] = $biological_status[0]->name;
    }

    // -countryOfOriginCode
    if (($country_of_origin = brapi_get_stock_property_values($stock->stock_id, 'CountryOfOriginCode'))
        && count($country_of_origin)) {
      $germplasm_data['countryOfOriginCode'] = $country_of_origin[0];
    }

    // -typeOfGermplasmStorageCode
    if (($type_of_storage = brapi_get_stock_property_values($stock->stock_id, 'TypeOfGermplasmStorageCode'))
        && count($type_of_storage)) {
      $germplasm_data['typeOfGermplasmStorageCode'] = $type_of_storage;
    }
    elseif (($type_of_storage = brapi_get_stock_cvterm_values($stock->stock_id, array('parent_term' => 'TypeOfGermplasmStorageCode', 'cv' => BRAPI_MULTICROP_PASSPORT_ONTOLOGY_CV)))
            && count($type_of_storage)) {
      $germplasm_data['typeOfGermplasmStorageCode'] = $type_of_storage[0]->name;
    }
    // else {
    //   $germplasm_data['typeOfGermplasmStorageCode'] = array();
    // }

    // -genus
    if (($genus = brapi_get_stock_property_values($stock->stock_id, 'genus'))
        && count($genus)) {
      $germplasm_data['genus'] = $genus[0];
    }
    else {
      $germplasm_data['genus'] = $stock->organism_id->genus;
    }

    // -species
    if (($species = brapi_get_stock_property_values($stock->stock_id, 'species'))
        && count($species)) {
      $germplasm_data['species'] = $species[0];
    }
    else {
      $germplasm_data['species'] = $stock->organism_id->species;
    }

    // -speciesAuthority
    if (($species_authority = brapi_get_stock_property_values($stock->stock_id, 'species authority'))
        && count($species_authority)) {
      $germplasm_data['speciesAuthority'] = $species_authority[0];
    }

    // -subtaxa
    if (($subtaxa = brapi_get_stock_property_values($stock->stock_id, 'subtaxa'))
        && count($subtaxa)) {
      $germplasm_data['subtaxa'] = $subtaxa[0];
    }

    // -subtaxaAuthority
    if (($subtaxa_authority = brapi_get_stock_property_values($stock->stock_id, 'subtaxa authority'))
        && count($subtaxa_authority)) {
    }

    // -donors
    $germplasm_data['donors'] = array();
    $donor_institute_code = brapi_get_stock_property_values($stock->stock_id, 'donor institute code');
    $donor_accession_number = brapi_get_stock_property_values($stock->stock_id, 'donor accession number');
    $donors_count = max(count($donor_institute_code), count($donor_accession_number));

    for ($i = 0; $i < $donors_count; ++$i) {
      $donor_data = array();
      if (isset($donor_institute_code[$i])) {
        // -donor institute code
        $donor_data['donorInstituteCode'] = $donor_institute_code[$i];
      }

      if (isset($donor_accession_number[$i])) {
        // -donor accession number
        $donor_data['donorAccessionNumber'] = $donor_accession_number[$i];
      }

      // $donor_data['germplasmPUI'] = NULL;

      $germplasm_data['donors'][] = $donor_data;
    }

    // -acquisitionDate
    if (($acquisition_date = brapi_get_stock_property_values($stock->stock_id, 'acquisition date'))
        && count($acquisition_date)
        && (1 < $acquisition_date[0])) {
      $germplasm_data['acquisitionDate'] = date('Ymd', $acquisition_date[0]);
    }

  }
  else {
    //+FIXME: error code stock not found
  }

  return brapi_output_json(array('result' => array('data' => array($germplasm_data))), $metadata, $debug_data);
}


/**
 * BRAPI: Germplasm pedigree
 *        brapi/v1/germplasm/\<id\>/pedigree
 *
 * GET: returns the pedigree of the given accession.
 *
 * Supported GET parameters:
 * - notation string
 *     The pedigree notation to use.
 *     Supported values: 'purdy'
 *     Default: purdy
 *     Example: 'notation=purdy'.
 *
 * Example:
 * - GET
 *   brapi/v1/germplasm/01BEL084609/pedigree?notation=purdy
 *
 * @return string
 *   a JSON string with the following variable (some can be embeded):
 *
 * Variable            Datatype   Required  Description
 *------------------------------------------------------
 * metadata            object            Y  pagination, status
 * pagination          object            Y  pageSize, currentPage, totalCount,
 *                                          totalPages
 * status              list              Y  code, message
 * result              object            Y  data
 * germplasmDbId       string            Y  Internal db identifier
 * defaultDisplayName  string            Y  A string representing the germplasm
 * pedigree            string            Y  Cross name with optional selection
 *                                          history.
 * parent1Id           string               germplasmDbId of parent1
 * parent2Id           string               germplasmDbId of parent2
 *
 * @ingroup brapi
 */
/*
 http://wheat.pw.usda.gov/ggpages/gopher/administration/Template%20for%20Germplasm%20records.html

 About pedigreeString:
 ---------------------

 The pedigree identifies the parents and crossing sequences used to produce the
 cultivar.  The method used to illustrate pedigrees is a slightly modified
 version of the system proposed by Purdy et al. in 1969 (see Crop Sci. 8:405-
 406).  Use of abbreviations has been minimized.  Crosses are symbolized by
 combinations of slash marks ("/") with female and male parents listed to left
 and right side, respectively.  Numbers indicate the order in which crosses
 were made:

                           /  = primary cross

                          /2/ = secondary cross

                          /3/ = tertiary cross

                          /X/ = Xth level cross, etc.

 Higher numbers indicate more recent crosses in the sequence.  The most recent
 or final cross used to create a cultivar is indicated by the highest number
 within the pedigree.  For example, the pedigree of "Scout" hard red winter
 wheat is:

      -------------------------------------------------------------------
      Example 1:  Comparison of Purdy pedigree nomenclature to a tree
                  diagram of the pedigree of Scout hard red winter wheat.

            Scout = Nebred /2/ Hope / Turkey Red /3/ Cheyenne / Ponca

                                       OR

                         Hope        Turkey Red
                           |______ / _____|
                                   |
                                   |
                Nebred            "?"
                   |_____ /2/ _____|
                           |              Cheyenne        Ponca
                           |                  |______ / ____|
                           |                          |
                           |                          |
                          "?"                        "?"
                           |___________ /3/ __________|
                                         |
                                         |
                                       Scout
      -------------------------------------------------------------------

 In narrative terms, an unidentified progeny of a primary cross between "Hope"
 hard red spring wheat and "Turkey Red" hard red winter wheat was selected and
 crossed to "Nebred" hard red winter wheat.  One of the progeny selected from
 the "Nebred/2/Hope/Turkey Red" sequence of crosses was crossed to another
 unidentified progeny derived by crossing "Cheyenne" and "Ponca" hard red
 winter wheats.  The cultivar Scout was selected from progeny resulting from
 the final or "/3/" cross.  Specific generations and selection techniques
 involved are not indicated, but may be obtained from the referenced
 literature.

 Single slash marks are also used where the parents are known, but the exact
 sequence of a series of crosses is unknown.  Backcrossing sequences are
 indicated by use of an asterisk ("*") preceded or followed by a number to
 indicate the total number of crosses made with the recurrent parent (see
 Examples 2 and 3).  Left and right parentheses are used to bracket both the
 pedigree and designation of breeding lines contained within a cultivar's
 pedigree (see Example 4).  Commas are used to separate breeding line pedigrees
 from designations within the parentheses.


      -------------------------------------------------------------------
      Example 2:  Pedigree with three backcrosses of female recurrent parent
                  for TAM 107 hard red winter wheat.

                            TAM 107 = TAM 105*4 / Amigo

                                         OR

                                                  TAM 105      Amigo
                                                     |____ / ____|
                                                           |
                                         TAM 105           |
         1st backcross>                     |____ *2 / ____|
                                                    |
                                  TAM 105           |
         2nd backcross>              |____ *3 / ____|
                                             |
                         TAM 105             |
         3rd backcross>     |_____ *4 / _____|
                                    |
                                    |
                                 TAM 107


      Example 3:  Pedigree with three backcrosses of male recurrent parent
                  for Blueboy II soft red winter wheat.

                      Blueboy II = Agent / Tascosa /2/ 4*Blueboy

                                          OR

               Agent      Tascosa
                 |____ / ____|
                       |
                       |          Blueboy
                       |____ /2/ ____|
                              |
                              |            Blueboy
          1st backcross>      |____ /2/ 2* ____|
                                       |
                                       |            Blueboy
          2nd backcross>               |____ /2/ 3* ____|
                                                |
                                                |            Blueboy
          3rd backcross>                        |____ /2/ 4* ____|
                                                         |
                                                         |
                                                     Blueboy II


      Example 4:  Use of parentheses to delineate breeding line used in the
                  pedigree of Pitic 62 hard red spring wheat.

            Pitic 62 = Yaktana 54 /2/ (Sel. 26-1c, Norin 10 / Brevor)
                                       ^
                                       Indicates Sel. 26-1c as the male
                                       parent of the highest order cross for
                                       Pitic 62, with its own pedigree of
                                       "Norin 10 / Brevor".
                                      OR

                                   Norin 10          Brevor
                                      |________ / ______|
                                                |
                                                |
                   Yaktana 54              Sel. 26-1c
                       |__________ /2/ _________|
                                    |
                                    |
                                 Pitic 62

      -------------------------------------------------------------------

 Narratives providing more detailed information are used where necessary for
 clarification.  Pedigrees of cultivars screened from another cultivar are
 listed as "pure line selections".  Pedigrees of cultivars phenotypically
 selected from mixtures or out-crosses in commercial fields are listed as
 "farmer selections" with the original source material identified wherever
 possible.

 * /
function brapi_v1_germplasm_pedigree_json($germplasm_id) {

  // get call parameters
  $args = drupal_get_query_parameters();

  $metadata = brapi_prepare_metadata(1);

  $debug_data = array();

  //+FIXME: handle notation
  $notation   = isset($args['notation']) ? $args['notation'] : 'purdy';

  // get corresponding stock
  $pedigree_data = array();
  $selector = array(
    'uniquename' => $germplasm_id,
  );

  if ($stock = chado_generate_var('stock', $selector)) {
    // we got a matching stock
    // -germplasmDbId
    $pedigree_data['germplasmDbId'] = $stock->uniquename; // required

    // -defaultDisplayName
    $germplasm_data['defaultDisplayName'] = $stock->name; // required

    // get its parents
    $table_options = array(
      'include_fk' => array(
        'type_id' => array(
          'cv_id' => 1,
        ),
        'object_id' => array(
        ),
      ),
    );
    $stock = chado_expand_var($stock, 'table', 'stock_relationship', $table_options);

    //$pedigree_data['debug'] = $stock->stock_relationship->subject_id;
    $rank0_pedigree = array();
    foreach ($stock->stock_relationship->subject_id as $relationship) {
      if (0 == $relationship->rank) {
        if ('female_parent' == $relationship->type_id->name) {
          $rank0_pedigree['parent1Id'] = $relationship->object_id->uniquename;
        }
        elseif ('male_parent' == $relationship->type_id->name) {
          $rank0_pedigree['parent2Id'] = $relationship->object_id->uniquename;
        }
      }
    }

    $rank0_pedigree['pedigreeString'] = '';
    if ($rank0_pedigree['parent1Id'] || $rank0_pedigree['parent2Id']) {
      $rank0_pedigree['pedigreeString'] = $rank0_pedigree['parent1Id'] . ' / ' . $rank0_pedigree['parent2Id'];
    }
    $pedigree_data = array(array_merge($pedigree_data, $rank0_pedigree));
  }

  return brapi_output_json(array('result' => $pedigree_data), $metadata, $debug_data);
}


/ **
 * BRAPI: Germplasm markerprofiles
 *        brapi/v1/germplasm/\<id\>/markerprofiles
 *
 * GET: returns the markerprofiles of the given accession.
 *
 * Example:
 * - GET
 *   brapi/v1/germplasm/01BEL084609/markerprofiles
 *
 * @return string
 *   a JSON string with the following variable (some can be embeded):
 *
 * Variable            Datatype   Required  Description
 *------------------------------------------------------
 * metadata            object            Y  pagination, status
 * pagination          object            Y  pageSize, currentPage, totalCount,
 *                                          totalPages
 * status              list              Y  code, message
 * result              object            Y  data
 * germplasmDbId       string               Internal db identifier
 * defaultDisplayName  string               A string representing the germplasm
 * markerProfiles      array of string      An array of marker profile
 *                                          identifiers
 *
 * @ingroup brapi
 * /
function brapi_v1_germplasm_markerprofiles_json($germplasm_id) {

  $metadata = brapi_prepare_metadata(1);

  $debug_data = array();

  // get corresponding stock
  $selector = array(
    'uniquename' => $germplasm_id,
  );
  if ($stock = chado_generate_var('stock', $selector)) {
    // get corresponding markers
    $markerprofiles_data = array();
    $query_arguments = array(
      ':stock_uniquename' => $germplasm_id,
    );

    $sql_query = "
        SELECT f.uniquename
        FROM
          feature f
            INNER JOIN feature_genotype fg USING (feature_id)
            INNER JOIN stock_genotype sg USING (genotype_id)
            INNER JOIN stock s USING (stock_id)
        WHERE
          s.uniquename = :stock_uniquename
    ;";
    $markerprofiles = chado_query($sql_query, $query_arguments);

    if (count($markerprofiles)) {
      // -germplasmDbId
      $markerprofiles_data['germplasmDbId'] = $stock->uniquename;
      // -defaultDisplayName
      $markerprofiles_data['defaultDisplayName'] = $stock->name;

      $markerprofiles_data['markerProfiles'] = array();
      foreach ($markerprofiles as $markerprofile) {
        $markerprofiles_data['markerProfiles'][] = $markerprofile->uniquename;
      }
    }
  }

  return brapi_output_json(array('result' => $markerprofiles_data), $metadata, $debug_data);
}


/ **
 * brapi_v1_crops_json()
 *
 * /
function brapi_v1_crops_json() {

  // init status and debug data
  $status = array();
  $debug_data = array();
  $metadata = brapi_prepare_metadata(0);

  $sql_count_query = "
    SELECT o.common_name AS \"crop\"
    FROM
      {stock} s
        INNER JOIN organism o USING (organism_id)
    WHERE
      s.is_obsolete = FALSE
    GROUP BY o.common_name
    ;";
  $crop_list_result = chado_query($sql_count_query);
  $crop_list = array();
  while ($crop_data = $crop_list_result->fetchAssoc()) {
    $crop_list[] = $crop_data['crop'];
  }

  return brapi_output_json(array('result' => $crop_list), $metadata, $debug_data);
}


/ **
 * brapi_v1_markerprofiles_json()
 *
 * /
function brapi_v1_markerprofiles_json() {
  return brapi_raise_error_json(501, "Not implemented!");
}


/ **
 * brapi_v1_markerprofiles_details_json()
 *
 * /
function brapi_v1_markerprofiles_details_json($markerprofile_id) {
  return brapi_raise_error_json(501, "Not implemented!");
}


/ **
 * brapi_v1_allelematrices_json()
 *
 * /
function brapi_v1_allelematrices_json($allelematrix_id) {
  return brapi_raise_error_json(501, "Not implemented!");
}


/ **
 * brapi_v1_traits_json()
 *
 * /
function brapi_v1_traits_json() {
  return brapi_raise_error_json(501, "Not implemented!");
}

/ **
 * brapi_v1_germplasm_list_by_trialdbid_json()
 *
 * /
function brapi_v1_germplasm_list_by_trialdbid_json() {
  return brapi_raise_error_json(501, "Not implemented!");
}

/ **
 * brapi_v1_attributes_json()
 *
 * /
function brapi_v1_attributes_json() {
  return brapi_raise_error_json(501, "Not implemented!");
}

/ **
 * brapi_v1_attribute_categories_json()
 *
 * /
function brapi_v1_attribute_categories_json() {
  return brapi_raise_error_json(501, "Not implemented!");
}

/ **
 * brapi_v1_germplasm_attributes_json()
 *
 * /
function brapi_v1_germplasm_attributes_json($germplasm_id) {
  //+++
  $attribute_cv = array('mgis_plant_general_appearance',
    'mgis_petiole_midrib_leaf',
    'mgis_pseudostem_suckers',
    'mgis_male_flower',
    'mgis_inflorescence_male_bud',
    'mgis_bract',
    'mgis_fruit',
  );

  // init status and debug data
  $status = array();
  $debug_data = array();
  $metadata = brapi_prepare_metadata(0);

  $sql_query = '
    SELECT
      sp.stockprop_id AS "attributeDbId",
      spcvt.name AS "attributeName",
      sp.value AS "value",
      sp.rank as "rank"
    FROM {stock} s
      JOIN {stockprop} sp ON (s.stock_id = sp.stock_id)
      JOIN {cvterm} spcvt ON (spcvt.cvterm_id = sp.type_id)
      JOIN {cv} cv ON (cv.cv_id = spcvt.cv_id)
    WHERE
      s.uniquename = :germplasm_id
      AND cv.name IN (:attribute_cv)
    ORDER BY "attributeName","rank"
    ;'
  ;
  $query_arguments = array(
    ':germplasm_id' => $germplasm_id,
    ':attribute_cv' => $attribute_cv,
  );

  $attribute_list_result = chado_query($sql_query, $query_arguments);
  $attribute_list = array();
  while ($attribute = $attribute_list_result->fetchAssoc()) {
    $attribute_list[] = $attribute;
  }

  return brapi_output_json(array('result' => array('germplasmDbId' => $germplasm_id, 'data' => $attribute_list)), $metadata, $debug_data);
}

/ **
 * brapi_v1_study_types_json()
 *
 * /
function brapi_v1_study_types_json() {
  return brapi_raise_error_json(501, "Not implemented!");
}

/ **
 * brapi_v1_study_instances_json()
 *
 * /
function brapi_v1_study_instances_json($study_id) {
  return brapi_raise_error_json(501, "Not implemented!");
}

/ **
 * brapi_v1_studies_json()
 *
 * /
function brapi_v1_studies_json() {
  // init status and debug data
  $status = array();
  $debug_data = array();

  //+FIXME: set CV terms
  $projectprop_year_type = 'year';
  $projectprop_location_type = 'location';

  // prepare count query
  $sql_count_query = "
    SELECT COUNT(DISTINCT p.project_id) AS \"count\"
    FROM
      {project} p
    ;"
  ;

  $project_count_result = chado_query($sql_count_query);
  $project_count = floor($project_count_result->fetchAssoc()['count']);
  $metadata = brapi_prepare_metadata($project_count);

  $pager = '';
  if ($project_count) {
    $pager = 'LIMIT ' . $metadata['pagination']['pageSize'] . "\n";

    if ($metadata['pagination']['currentPage'] > 0) {
      $pager .= 'OFFSET ' . (($metadata['pagination']['currentPage']-1)*$metadata['pagination']['pageSize']) . "\n";
    }
  }

  $sql_query = "
    SELECT
      p.project_id AS \"studyDbId\",
      p.name AS \"studyName\",
      (
         SELECT string_agg(ppyear.value, ',')
         FROM
          {projectprop} ppyear
            INNER JOIN {cvterm} cvtyear ON cvtyear.cvterm_id = ppyear.type_id AND cvtyear.name = '$projectprop_year_type'
         WHERE
           ppyear.project_id = p.project_id
         GROUP BY ppyear.rank
         ORDER BY ppyear.rank
      ) AS \"years\",
      pploc.projectprop_id AS \"locationDbId\"
    FROM {project} p
      LEFT JOIN {projectprop} pploc ON p.project_id = pploc.project_id
      INNER JOIN {cvterm} cvtloc ON pploc.type_id = cvtloc.cvterm_id AND cvtloc.name = '$projectprop_location_type'
    ORDER BY \"studyName\"
    $pager
    ;"
  ;

  $study_list_result = chado_query($sql_query);
  $study_list = array();
  while ($study = $study_list_result->fetchAssoc()) {
    if (!empty($study['years'])) {
      $study['years'] = explode(',', $study['years']);
    }
    $study_list[] = $study;
  }

  return brapi_output_json(array('result' => array('data' => $study_list)), $metadata, $debug_data);
}

/ **
 * brapi_v1_study_json()
 *
 * /
function brapi_v1_study_json($study_id) {
  return brapi_raise_error_json(501, "Not implemented!");
}

/ **
 * brapi_v1_study_details_json()
 *
 * /
function brapi_v1_study_details_json($study_id) {
  return brapi_raise_error_json(501, "Not implemented!");
}

/ **
 * brapi_v1_study_table_json()
 *
 * /
function brapi_v1_study_table_json($study_id) {
  $metadata = brapi_prepare_metadata(1);

  $debug_data = array();

  // get corresponding project
  $study_data = array();
  $selector = array(
    'project_id' => $study_id,
  );

  if ($project = chado_generate_var('project', $selector)) {
    // we got a matching project

    // check request type
    if ($_SERVER['REQUEST_METHOD'] === 'PUT') {
      //+FIXME: not logged in => 400, invalid_token
      if (!user_access(BRAPI_UPDATE_PERMISSION)) {
          // insufficient_scope
        return brapi_raise_error_json(401, "Access denied!");
      }
      $putdata_stream = fopen("php://input", "r");
      $user_data = '';
      while ($data = fread($putdata_stream, 1024)) {
        $user_data .= $data;
      }
      fclose($putdata_stream);
      $json_data = json_decode($user_data, TRUE);
      if (!isset($json_data) || !is_array($json_data)) {
        // invalid json data provided
        return brapi_raise_error_json(400, "Invalid JSON data!");
      }
      //+FIXME: implementation missing
      return brapi_raise_error_json(501, "Not implemented!");
    }
    elseif ($_SERVER['REQUEST_METHOD'] === 'DELETE') {
      if (!user_access(BRAPI_UPDATE_PERMISSION)) { // user_access('administer site configuration')
        return brapi_raise_error_json(401, "Access denied!");
      }
      //+FIXME: DELETE not supported yet
    }
    elseif (($_SERVER['REQUEST_METHOD'] !== 'GET')
        && ($_SERVER['REQUEST_METHOD'] !== 'POST')) {
      //+FIXME: Unsupported method not supported yet
    }

    $study_data['studyDbId']    = $project->project_id; // required
    $study_data['studyName']    = $project->name;  // required
    //+FIXME
    $study_data['years']        = array();  // required
    $study_data['locationDbId'] = '';  // required

  }
  else {
    //+FIXME: error code stock not found
  }

  return brapi_output_json(array('result' => array('data' => array($study_data))), $metadata, $debug_data);
}

/ **
 * brapi_v1_study_layout_json()
 *
 * /
function brapi_v1_study_layout_json() {
  return brapi_raise_error_json(501, "Not implemented!");
}

/ **
 * brapi_v1_study_plot_json()
 *
 * /
function brapi_v1_study_plot_json($study_id, $observation_variable) {
  return brapi_raise_error_json(501, "Not implemented!");
}

/ **
 * brapi_v1_phenotypes_json()
 *
 * /
function brapi_v1_phenotypes_json() {
  return brapi_raise_error_json(501, "Not implemented!");
}

/ **
 * brapi_v1_phenotype_metadata_json()
 *
 * /
function brapi_v1_phenotype_metadata_json() {
  return brapi_raise_error_json(501, "Not implemented!");
}

/ **
 * brapi_v1_phenotype_hierarchical_json()
 *
 * /
function brapi_v1_phenotype_hierarchical_json() {
  return brapi_raise_error_json(501, "Not implemented!");
}

/ **
 * brapi_v1_trait_details_json()
 *
 * /
function brapi_v1_trait_details_json($trait_id) {
  return brapi_raise_error_json(501, "Not implemented!");
}

/ **
 * brapi_v1_data_types_json()
 *
 * /
function brapi_v1_data_types_json() {
  return brapi_raise_error_json(501, "Not implemented!");
}

/ **
 * brapi_v1_variables_json()
 *
 * /
function brapi_v1_variables_json() {
  return brapi_raise_error_json(501, "Not implemented!");
}

/ **
 * brapi_v1_variable_detail_json()
 *
 * /
function brapi_v1_variable_detail_json($variable_id) {
  return brapi_raise_error_json(501, "Not implemented!");
}

/ **
 * brapi_v1_genome_map_json()
 *
 * /
function brapi_v1_genome_map_json() {
  return brapi_raise_error_json(501, "Not implemented!");
}

/ **
 * brapi_v1_genome_map_details_json()
 *
 * /
function brapi_v1_genome_map_details_json($map_id) {
  return brapi_raise_error_json(501, "Not implemented!");
}

/ **
 * brapi_v1_genome_map_data_json()
 *
 * /
function brapi_v1_genome_map_data_json($map_id) {
  return brapi_raise_error_json(501, "Not implemented!");
}

/ **
 * brapi_v1_genome_map_data_by_range_on_linkagegroup_json()
 *
 * /
function brapi_v1_genome_map_data_by_range_on_linkagegroup_json($map_id, $linkagegroup_id) {
  return brapi_raise_error_json(501, "Not implemented!");
}

/ **
 * brapi_v1_observation_levels_json()
 *
 * /
function brapi_v1_observation_levels_json() {
  return brapi_raise_error_json(501, "Not implemented!");
}

/ **
 * brapi_v1_collection_json()
 *
 * /
function brapi_v1_collection_json($study_id) {
  return brapi_raise_error_json(501, "Not implemented!");
}

/ **
 * brapi_v1_observation_sets_json()
 *
 * /
function brapi_v1_observation_sets_json($observation_id) {
  return brapi_raise_error_json(501, "Not implemented!");
}

/ **
 * brapi_v1_locations_json()
 *
 * /
function brapi_v1_locations_json() {
  return brapi_raise_error_json(501, "Not implemented!");
}

/ **
 * brapi_v1_programs_json()
 *
 * /
function brapi_v1_programs_json() {
  return brapi_raise_error_json(501, "Not implemented!");
}
*/
