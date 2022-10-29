<?php

/**
 * @file
 */

namespace Drupal\brapi\Controller;

use Drupal\Core\Controller\ControllerBase;
use Drupal\Core\Url;
use Drupal\user\Entity\User;
use Symfony\Component\HttpFoundation\Request;
use Symfony\Component\HttpFoundation\JsonResponse;
use Symfony\Component\HttpKernel\Exception\PreconditionFailedHttpException;
use Symfony\Component\HttpKernel\Exception\TooManyRequestsHttpException;

class BrapiController extends ControllerBase {

  /**
   * Main BrAPI page with basic informations.
   */
  public function mainPage() {
    // $links = [];
    // $links[] = [
    //   '#type' => 'link',
    //   '#url' => Url::fromRoute('theming_example.form_text'),
    //   '#title' => t('Simple form 2'),
    // ];
    // $content = [
    //   '#theme' => 'item_list',
    //   '#theme_wrappers' => ['theming_example_content_array'],
    //   '#items' => $links,
    //   '#title' => t('Some examples of pages and forms that are run through theme functions.'),
    // ];
    $content = [
      '#theme' => 'brapi_main',
      '#title' => t('BrAPI Endpoint Details.'),
    ];

    return $content;
  }

  /**
   * Displays BrAPI documentation.
   */
  public function documentationPage() {
    $content = [
      '#theme' => 'brapi_documentation',
      '#title' => t('BrAPI Documentation.'),
    ];

    return $content;
  }

  /**
   * Export BrAPI call results as JSON.
   *
   * Useful documentation:
   *
   *   - https://api.drupal.org/api/drupal/vendor!symfony!http-foundation!Request.php/class/Request/9.3.x
   *   - https://api.drupal.org/api/drupal/vendor%21symfony%21routing%21Route.php/class/Route/9.3.x
   *   - https://api.drupal.org/api/drupal/vendor%21symfony%21http-foundation%21ParameterBag.php/class/ParameterBag/9.3.x
   */
  public function brapiCall() {
    // Get intended HTTP method.
    $request = \Drupal::request();
    $method = strtolower($request->getMethod());
    $route = $request->attributes->get('_route_object');
    if (empty($route)) {
      \Drupal::logger('brapi')->error('No route object for "' . $request->getUri() . '"');
      throw new \Symfony\Component\HttpKernel\Exception\NotFoundHttpException();
    }
    if (!preg_match('#^/brapi/(v\d)(/.+)#', $route->getPath(), $matches)) {
      \Drupal::logger('brapi')->error('Invalid path structure for "' . $request->getUri() . '"');
      throw new \Symfony\Component\HttpKernel\Exception\NotFoundHttpException();
    }
    $version = $matches[1];
    $call = $matches[2];
    // Get current settings.
    $config = \Drupal::config('brapi.settings');
    $call_settings = $config->get('calls');

    // Check if we have something for that call and method.
    if (empty($call_settings[$version][$call][$method])) {
      \Drupal::logger('brapi')->error("No available settings for call '$call' ($version) using method $method.");
      throw new \Symfony\Component\HttpKernel\Exception\NotFoundHttpException();
    }

    // Make sure we got a definition.
    $active_def = $config->get($version . 'def');
    $brapi_def = brapi_get_definition($version, $active_def);
    if (empty($brapi_def['calls'][$call])) {
      \Drupal::logger('brapi')->error("No available definition for call '$call' ($version, $active_def) using method $method.");
      throw new \Symfony\Component\HttpKernel\Exception\NotFoundHttpException();
    }

    //@todo: first manages calls: /v*/search, /v2/lists,
    // Check if call works with one data type.
    if (1 == count($brapi_def['calls'][$call]['data_types'])) {
      $json_array = $this->processObjectCalls($request, $config, $version, $call, $method);
    }
    elseif (('v2' == $version) && ('/serverinfo' == $call)) {
      $json_array = $this->processV2ServerInfoCall($request, $config);
    }
    elseif (('v1' == $version) && ('/calls' == $call)) {
      $json_array = $this->processV1CallsCall($request, $config);
    }
    elseif (('v1' == $version) && ('/login' == $call)) {
      $json_array = $this->processV1LoginCall($request, $config);
    }
    elseif (('v1' == $version) && ('/logout' == $call)) {
      $json_array = $this->processV1LogoutCall($request, $config);
    }
    else {
      $this->logger('brapi')->warning('Unsupported call type: %call (%version)', ['%call' => $call, '%version' => $version, ]);
    }

    // @todo: Manage call cases.
    // -search calls
    //  Use Drupal cache to save search results \Drupal::cache('brapi_search')
    //  On search calls, generate a token based on the call name and a
    //  normalized structure of the search arguments (same order, etc.) in order
    //  to use a same cache token for as same query issued multiple times with
    //  the same arguments but in any order. Use MD5?
    //  An empty cache element must be created with the token_id.
    //  The search process must be launch in background with the token_id and
    //  the search arguments.
    //  When done, the search process should fill the cache using the token_id.
    //  On the "get results" part, the cache is fetched. If no cache found, it
    //  means the results are lost, if cache found but empty, it means still
    //  computing and if found with results, it's done.
    // -manage special output formats (eg. /phenotypes-search/csv)

    // https://api.drupal.org/api/drupal/vendor!symfony!http-foundation!Request.php/class/Request/9.3.x
    // https://api.drupal.org/api/drupal/vendor%21symfony%21routing%21Route.php/class/Route/9.3.x
    // https://api.drupal.org/api/drupal/vendor%21symfony%21http-foundation%21ParameterBag.php/class/ParameterBag/9.3.x
    // POST values:
    // $value = $request->request->get('param');
    // GET values:
    // $value = $request->query->get('param');

    // Check if the whole response is not specific and has not been set already.
    if (!isset($json_array)) {
      // @todo: Add error and debug info.
      $json_array = $this->generateMetadata($request, $config);
    }
    return new JsonResponse($json_array);
  }

  /**
   * Returns the optional POST data structure.
   *
   * @param \Symfony\Component\HttpFoundation\Request $request
   *   The HTTP request object.
   * @return ?array
   *   The posted JSON data structure or NULL.
   */
  function getPostData(Request $request) :?array {
    // Get optional POST JSON data.
    $content = $request->getContent();
    if (!empty($content)) {
      $json_input = json_decode($content, TRUE);
    }
    return $json_input;
  }

  /**
   * Generates metadata structure.
   *
   * @param \Symfony\Component\HttpFoundation\Request $request
   *   The HTTP request object.
   * @param \Drupal\Core\Config\ImmutableConfig $config
   *   BrAPI config.
   * @param array $parameters
   *   Metadata parameter structure.
   * @return array
   *   The generated metadata structure.
   */
  function generateMetadata(
    Request $request,
    \Drupal\Core\Config\ImmutableConfig $config,
    array $parameters = []
  ) {
    $status      = $parameters['status'] ?? [];
    $datafiles   = $parameters['datafiles'] ?? [];
    $page_size   = $parameters['page_size'] ?? BRAPI_DEFAULT_PAGE_SIZE;
    $page        = $parameters['page'] ?? 0;
    $total_count = $parameters['total_count'] ?? 1;
    // Ajust pagination.
    if (1 > $page_size) {
      $page_size = BRAPI_DEFAULT_PAGE_SIZE;
    }
    if (0 > $total_count) {
      $total_count = 0;
    }
    // Auto-compute page count according to page size and total count.
    if (empty($parameters['total_pages'])) {
      $total_pages = ceil($total_count/$page_size);
    }
    else {
      $total_pages = $parameters['total_pages'];
    }
    // Adjust current page index if needed.
    if ($page >= $total_pages) {
      // Note: result might be -1.
      $page = $total_pages - 1;
    }
    if (0 > $page) {
      $page = 0;
    }

    $json_array = [
      'metadata' => [
          'status' => $status,
          'pagination' => [
              'pageSize' => $page_size,
              'currentPage' => $page,
              'totalCount' => $total_count,
              'totalPages' => $total_pages,
          ],
          'datafiles' => $datafiles,
      ],
    ];

    return $json_array;
  }

  /**
   * V2 /serverinfo call.
   *
   * Generates /serverinfo response.
   *
   * @param \Symfony\Component\HttpFoundation\Request $request
   *   The HTTP request object.
   * @param \Drupal\Core\Config\ImmutableConfig $config
   *   BrAPI config.
   * @return array
   *   The response data structure.
   */
  function processV2ServerInfoCall(
    Request $request,
    \Drupal\Core\Config\ImmutableConfig $config
  ) {
    // Get verssions supporting each call through BrAPI definitions.
    // $brapi_def = brapi_get_definition($version, $active_def);
    $active_def = $config->get('v2def');
    $call_settings = $config->get('calls');
    $calls = [];
    foreach ($call_settings['v2'] as $call => $methods) {
      $methods = array_map('strtoupper', array_keys(array_filter($methods)));
      $calls[] = [
        'contentTypes' => [BRAPI_MIME_JSON],
        'dataTypes'    => [BRAPI_MIME_JSON],
        'methods'      => $methods,
        'service'      => substr($call, 1),
        'versions'     => [$active_def],
      ];
    }
    $metadata = $this->generateMetadata($request, $config);
    $result = [
      'result' => [
        'contactEmail'      => $config->get('contact_email') ?? '',
        'documentationURL'  => $config->get('documentation_url') ?? '',
        'location'          => $config->get('location') ?? '',
        'organizationName'  => $config->get('organization_name') ?? '',
        'organizationURL'   => $config->get('organization_url') ?? '',
        'serverDescription' => $config->get('server_description') ?? '',
        'serverName'        => $config->get('server_name') ?? '',
        'calls'             => $calls,
      ],
    ];
    return $metadata + $result;
  }

  /**
   * V1 /calls call.
   *
   * Generates /calls response.
   *
   * @param \Symfony\Component\HttpFoundation\Request $request
   *   The HTTP request object.
   * @param \Drupal\Core\Config\ImmutableConfig $config
   *   BrAPI config.
   * @return array
   *   The response data structure.
   */
  function processV1CallsCall(
    Request $request,
    \Drupal\Core\Config\ImmutableConfig $config
  ) {
    // Get verssions supporting each call through BrAPI definitions.
    // $brapi_def = brapi_get_definition($version, $active_def);
    $active_def = $config->get('v1def');
    $call_settings = $config->get('calls');
    $calls = [];
    foreach ($call_settings['v1'] as $call => $methods) {
      $methods = array_map('strtoupper', array_keys(array_filter($methods)));
      $calls[] = [
        'call'         => substr($call, 1),
        'dataTypes'    => [BRAPI_MIME_JSON],
        'datatypes'    => [BRAPI_MIME_JSON],
        'methods'      => $methods,
        'versions'     => [$active_def],
      ];
    }
    $metadata = $this->generateMetadata($request, $config);
    $result = ['result' => ['data' => $calls]];
    return $metadata + $result;
  }

  /**
   * V1 /login call.
   *
   * Manages /login call.
   *
   * @param \Symfony\Component\HttpFoundation\Request $request
   *   The HTTP request object.
   * @param \Drupal\Core\Config\ImmutableConfig $config
   *   BrAPI config.
   * @return array
   *   The response data structure.
   * @throw \Symfony\Component\HttpKernel\Exception\TooManyRequestsHttpException
   * @throw \Symfony\Component\HttpKernel\Exception\PreconditionFailedHttpException
   */
  function processV1LoginCall(
    Request $request,
    \Drupal\Core\Config\ImmutableConfig $config
  ) {
    // Enforce HTTPS use.
    if (!$request->isSecure()) {
      throw new PreconditionFailedHttpException(
        'You must use HTTPS protocol to login. Login through unsecure HTTP is forbidden.'
      );
    }

    // Default.
    $result = [
      'access_token'    => '',
      'expires_in'      => -1,
      'userDisplayName' => \Drupal::currentUser()->getDisplayName(),
      'client_id'       => \Drupal::currentUser()->getAccountName(),
    ];

    $json_input = $this->getPostData($request);
    // Check for log in request.
    $method = strtolower($request->getMethod());
    if ('post' == $method) {
      // Get user log in data.
      $password = trim($json_input['password'] ?? '');
      $name = trim($json_input['username'] ?? $json_input['client_id'] ?? '');

      if (!empty($name) && strlen($password) > 0) {
        // Do not allow any login from the current user's IP if the limit has been
        // reached. Default is 50 failed attempts allowed in one hour. This is
        // independent of the per-user limit to catch attempts from one IP to log
        // in to many different user accounts.  We have a reasonably high limit
        // since there may be only one apparent IP for all users at an institution.
        $user_flood_control = \Drupal::service('user.flood_control');
        $flood_config = \Drupal::config('user.flood');
        if (!$user_flood_control
          ->isAllowed('user.failed_login_ip', $flood_config
          ->get('ip_limit'), $flood_config
          ->get('ip_window'))
        ) {
          throw new TooManyRequestsHttpException('Too many attempts.');
        }

        $account = user_load_by_name($name);
        if ($account) {
          if ($flood_config->get('uid_only')) {
            // Register flood events based on the uid only, so they apply for any
            // IP address. This is the most secure option.
            $identifier = $account->id();
          }
          else {
            // The default identifier is a combination of uid and IP address. This
            // is less secure but more resistant to denial-of-service attacks that
            // could lock out all users with public user names.
            $identifier = $account->id() . '-' . $request->getClientIp();
          }

          // Don't allow login if the limit for this user has been reached.
          // Default is to allow 5 failed attempts every 6 hours.
          if (!$user_flood_control
            ->isAllowed('user.failed_login_user', $flood_config
            ->get('user_limit'), $flood_config
            ->get('user_window'), $identifier)
          ) {
            throw new TooManyRequestsHttpException('Too many attempts.');
          }
        }

        // We are not limited by flood control, so try to authenticate.
        $uid = \Drupal::service('user.auth')->authenticate($name, $password);
      }
      if (!empty($uid)) {
        user_login_finalize($account);
        // Generate a new token.
        $token_id = bin2hex(random_bytes(16));
        $cid = 'brapi:' . $token_id;
        $data = ['username' => $name];
        $maxlifetime = 86400;
        $expiration = time() + $maxlifetime;
        \Drupal::cache('brapi_token')->set($cid, $data, $expiration);
        $result = [
          'access_token'    => $token_id,
          'expires_in'      => $maxlifetime,
          'userDisplayName' => $account->getDisplayName(),
          'client_id'       => $account->getAccountName(),
        ];
      }
      elseif (\Drupal::currentUser()->id()) {
        // Logout as the login try failed.
        user_logout();
      }
    }
    $metadata = $this->generateMetadata($request, $config);
    return $metadata + $result;
  }

  /**
   * V1 /logout call.
   *
   * Manages /logout call.
   *
   * @param \Symfony\Component\HttpFoundation\Request $request
   *   The HTTP request object.
   * @param \Drupal\Core\Config\ImmutableConfig $config
   *   BrAPI config.
   * @return array
   *   The response data structure.
   */
  function processV1LogoutCall(
    Request $request,
    \Drupal\Core\Config\ImmutableConfig $config
  ) {
    user_logout();
    return [];
  }

  /**
   * Data object request call.
   *
   * Manages single object and object list callls.
   *
   * @param \Symfony\Component\HttpFoundation\Request $request
   *   The HTTP request object.
   * @param \Drupal\Core\Config\ImmutableConfig $config
   *   BrAPI config.
   * @return array
   *   The response data structure.
   */
  function processObjectCalls(
    Request $request,
    \Drupal\Core\Config\ImmutableConfig $config,
    $version,
    $call,
    $method
  ) {
    $page_size   = 1;
    $page        = 0;
    $total_count = 1;
    $result = [];

    $active_def = $config->get($version . 'def');
    $brapi_def = brapi_get_definition($version, $active_def);

    // Get URL parameter from route placeholder (object identifier).
    $filters = $request->attributes->get('_raw_variables')->all();
    $single_record = FALSE;
    // @todo: also check for sub-calls (ex. /seedlots/{seedLotDbId}/transactions).
    if (!empty($filters)) {
      // An identifier has been provided, will return 1 record at most.
      $single_record = TRUE;
      $page = 0;
      $page_size = 1;
    }
    else {
      // No filter here means no identifier provided in the URL.
      // We will return a page of objects, check for pagintation.
      $page = $request->query->get('page') ?? 0;
      if (!empty($page)) {
        $filters['#page'] = $page;
      }
      $page_size = $request->query->get('pageSize') ?? BRAPI_DEFAULT_PAGE_SIZE;
      if (!empty($page_size)) {
        $filters['#pageSize'] = $page_size;
      }
    }

    // Get associated content.
    $mapping_loader = \Drupal::service('entity_type.manager')->getStorage('brapidatatype');
    $datatype_id = brapi_generate_datatype_id(array_keys($brapi_def['calls'][$call]['data_types'])[0], $version, $active_def);
    $datatype_mapping = $mapping_loader->load($datatype_id);
    if (empty($datatype_mapping)) {
      \Drupal::logger('brapi')->error("No mapping available for data type '$datatype_id'.");
      throw new \Symfony\Component\HttpKernel\Exception\NotFoundHttpException();
    }
    // Process query filters.
    foreach ($brapi_def['calls'][$call]['definition'][$method]['parameters'] as $filter_param) {
      $param_name = $filter_param['name'];
      // Only take into account parameters in URL query string and skip
      // output control parameters.
      if (('query' != $filter_param['in'])
          || (in_array($param_name, ['page', 'pageSize', 'Authorization']))
      ) {
        continue 1;
      }
      $param_value = $request->query->get($param_name);
      if ((NULL != $param_value) && ($param_value != '')) {
        $filters[$param_name] = $param_value;
      }
    }

    // Fetch BrAPI object(s).
    $brapi_data = $datatype_mapping->getBrapiData($filters);
    $entities = $brapi_data['entities'];

    // $result not set: regular data call.
    if ($single_record) {
      if (1 < count($entities)) {
        // Warn that more than one corresponding record was found.
        $route = $request->attributes->get('_route_object');
        \Drupal::logger('brapi')->warning(
          "%count records found wil expecting only one for call %call.",
          ['%count' => count($entities), '%call' => $route->getPath(), ]
        );
      }
      $result = ['result' => ['data' => $entities[0]]];
    }
    else {
      $result = ['result' => ['data' => $entities]];
    }

    // Update $total_count and $total_pages.
    $total_count = $brapi_data['total_count'];

    $parameters = [
      'page_size'   =>  $page_size,
      'page'        =>  $page,
      'total_count' =>  $total_count,
    ];

    $metadata = $this->generateMetadata($request, $config, $parameters);
    return $metadata + $result;
  }
}
