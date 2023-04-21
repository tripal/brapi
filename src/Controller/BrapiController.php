<?php

/**
 * @file
 */

namespace Drupal\brapi\Controller;

use Drupal\brapi\Entity\BrapiList;
use Drupal\brapi\Entity\BrapiToken;
use Drupal\Core\Config\ImmutableConfig;
use Drupal\Core\Controller\ControllerBase;
use Drupal\Core\Url;
use Drupal\user\Entity\User;
use Symfony\Component\HttpFoundation\Request;
use Symfony\Component\HttpFoundation\JsonResponse;
use Symfony\Component\HttpKernel\Exception\AccessDeniedHttpException;
use Symfony\Component\HttpKernel\Exception\BadRequestHttpException;
use Symfony\Component\HttpKernel\Exception\NotFoundHttpException;
use Symfony\Component\HttpKernel\Exception\PreconditionFailedHttpException;
use Symfony\Component\HttpKernel\Exception\TooManyRequestsHttpException;
use Symfony\Component\HttpKernel\Exception\UnauthorizedHttpException;

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
   * Displays BrAPI token page.
   */
  public function tokenPage() {
    $content = [
      '#theme' => 'brapi_token',
      '#title' => t('User Access Token.'),
    ];

    return $content;
  }

  /**
   * Generates a new token if needed and displays BrAPI token page.
   */
  public function newTokenPage() {
    $token = BrapiToken::getUserToken(NULL, TRUE);
    return $this->tokenPage();
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
    // @otod: catch all exceptions and return a JSON error instead of Drupal.
    try {
      // Get intended HTTP method.
      $request = \Drupal::request();
      $method = strtolower($request->getMethod());
      $route = $request->attributes->get('_route_object');
      if (empty($route)) {
        $message = 'No route object for "' . $request->getUri() . '"';
        \Drupal::logger('brapi')->error($message);
        throw new NotFoundHttpException($message);
      }
      if (!preg_match('#^/brapi/(v\d)(/.+)#', $route->getPath(), $matches)) {
        $message = 'Invalid path structure for "' . $request->getUri() . '"';
        \Drupal::logger('brapi')->error($message);
        throw new NotFoundHttpException($message);
      }
      $version = $matches[1];
      $call = $matches[2];
      // Get current settings.
      $config = \Drupal::config('brapi.settings');
      $call_settings = $config->get('calls');

      // Check if we have something for that call and method.
      if (empty($call_settings[$version][$call][$method])) {
        $message = "No available settings for call '$call' ($version) using method $method.";
        \Drupal::logger('brapi')->error($message);
        throw new NotFoundHttpException($message);
      }

      // Make sure we got a definition.
      $active_def = $config->get($version . 'def');
      $brapi_def = brapi_get_definition($version, $active_def);
      if (empty($brapi_def['calls'][$call])) {
        $message = "No available definition for call '$call' ($version, $active_def) using method $method.";
        \Drupal::logger('brapi')->error($message);
        throw new NotFoundHttpException($message);
      }

      // Check permission.
      if ((!\Drupal::currentUser()->hasPermission(BRAPI_USE_PERMISSION))
          && !(('v1' == $version) && ('/login' == $call))
      ) {
        throw new AccessDeniedHttpException('You are not allowed to use BrAPI. Please use a valid access token.');
      }
      //@todo: also check write access on PUT or POST calls.

      // Manage call cases.
      if (0 === strpos($call, '/search/')) {
        $json_array = $this->processSearchCalls($request, $config, $version, $call, $method);
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
      elseif ('/lists' == $call) {
        // Special case of '/lists' thats uses 2 data types. The right one is
        // selected in processObjectCalls() by a dedicated "if".
        $json_array = $this->processObjectCalls($request, $config, $version, $call, $method);
      }
      elseif (1 == count($brapi_def['calls'][$call]['data_types'])) {
        // Call works with one data type: a regular BrAPI object call.
        $json_array = $this->processObjectCalls($request, $config, $version, $call, $method);
      }
      else {
        \Drupal::logger('brapi')->warning('Unsupported call type: %call (%version)', ['%call' => $call, '%version' => $version, ]);
      }

      // @todo: Manage other special call cases.
      // -manage special output formats (eg. /phenotypes-search/csv)

      // https://api.drupal.org/api/drupal/vendor!symfony!http-foundation!Request.php/class/Request/9.3.x
      // https://api.drupal.org/api/drupal/vendor%21symfony%21routing%21Route.php/class/Route/9.3.x
      // https://api.drupal.org/api/drupal/vendor%21symfony%21http-foundation%21ParameterBag.php/class/ParameterBag/9.3.x

      // Check if the whole response is not specific and has not been set already.
      if (!isset($json_array)) {
        // @todo: Add error and debug info.
        $parameters = [
          'status' => [[
            'message'     => 'Not implemented',
            'messageType' => 'ERROR',
          ]],
        ];
        $metadata = $this->generateMetadata($request, $config, $parameters);
        $json_array = $metadata;
      }
      $response = new JsonResponse($json_array);
      // Check for specified status code.
      if (!empty($json_array['metadata']['status']['code'])) {
        // Here we could delete the 'code' key from the status array if we want.
        $response->setStatusCode($json_array['metadata']['status']['code']);
      }
    }
    catch (\Symfony\Component\HttpKernel\Exception\HttpExceptionInterface $e) {
      // Catch HTTP errors.
      $parameters = [
        'status' => [[
          'message'     => $e->getMessage() ?: 'An exception occurred.',
          'messageType' => 'ERROR',
        ]],
      ];
      $json_array = $this->generateMetadata($request, $config, $parameters);
      $response = new JsonResponse($json_array);
      $response->setStatusCode($e->getStatusCode());
    }
    return $response;
  }

  /**
   * Returns the optional POST data structure.
   *
   * @param \Symfony\Component\HttpFoundation\Request $request
   *   The HTTP request object.
   * @param bool $raise_error
   *   If TRUE and an invalid JSON data has been provided, throws a
   *   BadRequestHttpException exception. Default: FALSE.
   * @param string $error_message
   *   Specific error message to display if needed. Default provided.
   * @return ?array
   *   The posted JSON data structure or NULL.
   *
   * @throws \Symfony\Component\HttpKernel\Exception\BadRequestHttpException
   */
  public function getPostData(
    Request $request,
    bool $raise_error = FALSE,
    string $error_message = 'Invalid or malformed JSON data.'
  ) :?array {
    // Get optional POST JSON data.
    $content = $request->getContent();
    if (!empty($content)) {
      $json_input = json_decode($content, TRUE);
    }
    if ($raise_error && !isset($json_input)) {
      $message = $error_message . "\n" . json_last_error_msg();
      \Drupal::logger('brapi')->error($message);
      throw new BadRequestHttpException($message);
    }
    return $json_input;
  }

  /**
   * Check given pageSize parameter and return an allowed pageSize value.
   *
   * @param \Drupal\Core\Config\ImmutableConfig $config
   *   BrAPI config.
   * @param ?string $page_size_param
   *   The pageSize parameter value to check.
   * @return int
   *   A valid pageSize value.
   */
  public function getCleanPageSize(
    ImmutableConfig $config,
    ?string $page_size_param = NULL
  ) :int {
    $page_size = $page_size_param ?? $config->get('page_size') ?? BRAPI_DEFAULT_PAGE_SIZE;
    // Ajust pagination.
    if (1 > $page_size) {
      // Null or negative page size, use default.
      $page_size = $config->get('page_size') ?? BRAPI_DEFAULT_PAGE_SIZE;
    }
    elseif (!empty($config->get('page_size_max'))
      && ($config->get('page_size_max') < $page_size)
    ) {
      // Limit to max (specified by the settings).
      $page_size = $config->get('page_size_max');
    }
    return intval($page_size);
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
  public function generateMetadata(
    Request $request,
    ImmutableConfig $config,
    array $parameters = []
  ) {
    $status = $parameters['status'] ?? [[
      'message'     => 'Request accepted, response successful',
      'messageType' => 'INFO',
    ]];
    $datafiles   = $parameters['datafiles'] ?? [];
    $page_size   = $this->getCleanPageSize($config, $parameters['page_size'] ?? NULL);
    $page        = $parameters['page'] ?? 0;
    $total_count = $parameters['total_count'] ?? 1;
    // Ajust pagination.
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
  public function processV2ServerInfoCall(
    Request $request,
    ImmutableConfig $config
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
  public function processV1CallsCall(
    Request $request,
    ImmutableConfig $config
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
  public function processV1LoginCall(
    Request $request,
    ImmutableConfig $config
  ) {
    // Enforce HTTPS use.
    if (!$request->isSecure() && !$config->get('insecure')) {
      throw new PreconditionFailedHttpException(
        'You must use HTTPS protocol to login. Login through insecure HTTP is forbidden.'
      );
    }

    // Default.
    $result = [
      'access_token'    => '',
      'expires_in'      => -1,
      'userDisplayName' => \Drupal::currentUser()->getDisplayName(),
      'client_id'       => \Drupal::currentUser()->getAccountName(),
    ];

    $json_input = $this->getPostData($request, TRUE);
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
          \Drupal::logger('brapi')->error('Too many login attempts for account "' . $name . '"');
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
            \Drupal::logger('brapi')->error('Too many login attempts for account "' . $name . '"');
            throw new TooManyRequestsHttpException('Too many attempts.');
          }
        }

        // We are not limited by flood control, so try to authenticate.
        $uid = \Drupal::service('user.auth')->authenticate($name, $password);
      }
      if (!empty($uid)) {
        user_login_finalize($account);
        $token = BrapiToken::getUserToken($account, TRUE);
        $result = [
          'access_token'    => $token->token->getString(),
          'expires_in'      => $token->expiration->getString(),
          'userDisplayName' => $account->getDisplayName(),
          'client_id'       => $account->getAccountName(),
        ];
      }
      elseif (\Drupal::currentUser()->id()) {
        // Logout current user as the login try failed.
        user_logout();
        throw new UnauthorizedHttpException('Basic', 'Login failed. Login out current user.');
      }
      else {
        // Authentication failed.
        throw new UnauthorizedHttpException('Basic', 'Login failed.');
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
  public function processV1LogoutCall(
    Request $request,
    ImmutableConfig $config
  ) {
    user_logout();
    return [];
  }

  /**
   * Manage search calls.
   *
   * This method uses `processObjectCalls` to search and fetch objects.
   * However, it can manage deferred search in order to send a quick response to
   * the client.
   *
   * Several strategies were considered to launch background searches:
   * - launching an external (shell) command. This approach must be used by both
   *   "cron" or "shell exec" methods. One problem is to pass the search query
   *   and the optional user token. It could be achieved through a file that
   *   must remain confidential (private file system). It adds the task of
   *   managing search files and handle race conditions. It also adds the
   *   question of managing crashed/lost computations.
   *   Then, executing such a command requiers Drupal system: the best option
   *   would be to use Drush and add it as a dependency for BrAPI module.
   *   Moreover, the external process must be "detached" from parent process
   *   and not be killed when the parent process ends.
   * - perform the search task when the search call is submitted. This approach
   *   must be used by "shell exec" or "fork" or "kernel.terminate event"
   *   methods. In any case, it must not block or delay the (202) response to
   *   the client.
   *   One problem is that it will use a PHP-FPM thread during the whole
   *   computation. To mitigate that problem, a limit of runnable paralele
   *   searches must be managed in order to avoid DoS.
   * - the Drupal cron method. Using the Drupal cron would not be conveninent as
   *   it could be launched at an hour basis while the computation could be
   *   short. It would mean the client would have to wait a very long time for
   *   almost nothing.
   * - a custom BrAPI cron method. It would require additional settings external
   *   to Drupal. It would require server shell access and/or being setup by an
   *   admin. The cron frequency might be hard to choose between too frequent
   *   and not enough frequent.
   * - fork method. The process is forked so parent process can return a search
   *   identifier to the client while the child process performs the search.
   *   However, forking is not convenient as handlers are shared, especially
   *   database handles, that leads to many types of problems. Also both
   *   process must not kill each other.
   * - exec method. A process is launched in background with the problems
   *   specified above (passing parameters, concurrency, dependencies, etc.).
   * - the symphony "kernel.terminate" event. That's the best option. The only
   *   drawback is that it locks a PHP-FPM process as described above, which can
   *   be mitigated by limiting the number of concurrent searches. Limiting
   *   could also be seen as a nice security feature.
   *
   * The symphony "kernel.terminate" event is the selected option. When a
   * deferred search is submitted by a client, a hash is generated from the
   * (normalized) request in order to identify a same search request submitted
   * multiple times. This identifier is used to create a Drupal cache item that
   * will be used as "search lock" to avoid running the same search multiple
   * times (the current implementation still allows race conditions but it is
   * mitigated by the fact that they have few chances to occur and will serve
   * the same results anyway). The cache item contains a "202" HTTP code until
   * the search is done. When the search completes, it stores its result in the
   * cache item. Every client query submitting the same search or querying the
   * search identifier will use that cache item to track the search status and
   * finally return the search results when available. When the cache item
   * expires, a client query with the old search identifier will result in a 404
   * "not found" response and a new search with the same parameters will create
   * a new cache item and perform a new search.
   *
   * Search must be managed "by users": clients must not be able to share the
   * same results unless the are anonymous or use the same user idenfier as user
   * may have different data access priviledges.
   *
   * @param \Symfony\Component\HttpFoundation\Request $request
   *   The HTTP request object.
   * @param \Drupal\Core\Config\ImmutableConfig $config
   *   BrAPI config.
   * @return array
   *   The response data structure.
   *
   * @see https://symfony.com/doc/current/components/http_kernel.html#component-http-kernel-kernel-terminate
   */
  public function processSearchCalls(
    Request $request,
    ImmutableConfig $config,
    $version,
    $call,
    $method
  ) {
    // Prepare pagger.
    $page_size   = 1;
    $page        = 0;
    $total_count = 1;
    $status      = [];

    // Check if the search call should use deferred result.
    $call_settings = $config->get('calls');
    if (!empty($call_settings[$version][$call]['deferred'])
        || str_contains($call, 'searchResultsDbId')
    ) {
      // Deferred.
      $status[] = [
        'message'     => 'Request accepted, response successful',
        'messageType' => 'INFO',
      ];

      // Check if a search identifier has been provided.
      $filters = $request->attributes->get('_raw_variables')->all();
      // Check call consistency.
      if (('get' == $method) && empty($filters['searchResultsDbId'])) {
        // Missing search identifier!
        $message = "No search identifier provided for call $call ($version).";
        \Drupal::logger('brapi')->error($message);
        throw new NotFoundHttpException($message);
      }
      elseif (('get' != $method) && !empty($filters['searchResultsDbId'])) {
        // Using a search identifier with a POST method.
        $message = "A search identifier has provided for call $call ($version) while not using the GET method.";
        \Drupal::logger('brapi')->error($message);
        throw new NotFoundHttpException($message);
      }

      if (!empty($filters['searchResultsDbId'])) {
        $new_search = FALSE;
        // Try to fetch a search result.
        $search_id = $filters['searchResultsDbId'];
      }
      else {
        // Process a new search.
        $new_search = TRUE;
        // Get a normalized search parameter array.
        // It will be used to generate a search identifier.
        $json_input = $this->getPostData($request, TRUE, "Invalid JSON data for search call $call ($version).");
        $filter_array_recursive = 'throw';
        $filter_array_recursive = function (&$array)
          use (&$filter_array_recursive)
        {
          foreach ($array as $key => $item) {
            if (is_array($item)) {
              $array[$key] = $filter_array_recursive($item);
              if (0 == count($array[$key])) {
                unset($array[$key]);
              }
            }
            elseif (!isset($array[$key]) || ('' === $array[$key])) {
              unset($array[$key]);
            }
          }
          ksort($array);
          return $array;
        };
        $json_input = $filter_array_recursive($json_input);
        // @todo: take into account user identifier for access restrictions and
        // concurrent search limitations.
        // Generate a normalized cache identifier (unique for a given search call
        // with a given set of parameter values).
        $search_id = md5($call . serialize($json_input));
      }
      $cid = 'brapi_search:' . $search_id;

      // Check if the search has already run.
      $cache_data = \Drupal::cache('brapi_search')->get($cid);
      if (!$cache_data) {
        if ($new_search) {
          // No such search in the cache, generate a new one and return the
          // corresponding search identifier.
          // Set search result lifetime.
          $max_life_time =
            $config->get('search_default_lifetime')
            ?? BRAPI_DEFAULT_SEARCH_LIFETIME
          ;
          // @todo: check for too many search jobs.
          $expiration = time() + $max_life_time;
          // Set a string as temporary value to be replaced by a result array.
          \Drupal::cache('brapi_search')->set($cid, ['metadata' => ['code' => 202,]], $expiration, ['brapi']);
          // Return 202 HTTP code.
          $status['code'] = 202;
          $result = ['searchResultsDbId' => $search_id, ];
          // Launch the search in background.
          $async_search = \Drupal::Service('brapi.async_search');
          $async_search->addSearch([
            'controller' => &$this,
            'request'    => $request,
            'config'     => $config,
            'version'    => $version,
            'call'       => $call,
            'method'     => $method,
            'cid'        => $cid,
            'expiration' => $expiration,
          ]);
        }
        else {
          // Search result expired or invalid search identifier.
          $message = "Invalid search identifier for call $call ($version).";
          \Drupal::logger('brapi')->error($message);
          throw new NotFoundHttpException($message);
        }
      }
      else {
        // Check cache content.
        if (!is_array($cache_data->data)) {
          // Invalid data. It should be an array.
          $message = "Corrupted search data for call $call ($version).";
          \Drupal::logger('brapi')->error($message);
          throw new NotFoundHttpException($message);
        }
        elseif (202 == $cache_data->data['metadata']['code']) {
          // Still searching.
          $page_size = 1;
          $status['code'] = 202;
          $result = ['searchResultsDbId' => $search_id, ];
        }
        elseif ((200 == ($cache_data->data['metadata']['code'] ?? 200))
            && array_key_exists('result', $cache_data->data)
        ) {
          // Search ended and we got something to return.
          // @todo: manage search result storage strategies:
          //   save query filters or save resulting identifiers as list
          //   or save the full result set?
          $result = ['result' => $cache_data->data['result']];
          // @todo: Manage pager.
          $page_size = $this->getCleanPageSize(
            $config,
            $request->query->get('pageSize')
          );
        }
        else {
          // Invalid result.
          $message = "Invalid search result for call $call ($version).";
          \Drupal::logger('brapi')->error($message);
          throw new NotFoundHttpException($message);
        }
      }

      $parameters = [
        'page_size'   => $page_size,
        'page'        => $page,
        'total_count' => $total_count,
        'status'      => $status ?: NULL,
      ];

      $metadata = $this->generateMetadata($request, $config, $parameters);
      return $metadata + $result;
    }
    elseif ('post' == $method) {
      // Direct execution.
      return $this->processObjectCalls($request, $config, $version, $call, $method);
    }
    else {
      // Using 'get' method on a non-deferred search call.
      $message = "Non-deferred search call must use POST method.";
      \Drupal::logger('brapi')->error($message);
      throw new NotFoundHttpException($message);
    }
  }

  /**
   * Data object request call.
   *
   * Manages single object and object list calls.
   *
   * @param \Symfony\Component\HttpFoundation\Request $request
   *   The HTTP request object.
   * @param \Drupal\Core\Config\ImmutableConfig $config
   *   BrAPI config.
   * @return array
   *   The response data structure.
   */
  public function processObjectCalls(
    Request $request,
    ImmutableConfig $config,
    $version,
    $call,
    $method
  ) {
    $page_size   = 1;
    $page        = 0;
    $total_count = 1;
    $status      = [];
    $result = [];

    $call_settings = $config->get('calls');
    $active_def = $config->get($version . 'def');
    $brapi_def = brapi_get_definition($version, $active_def);

    // Get URL parameter from route placeholder (object identifier).
    $filters = $request->attributes->get('_raw_variables')->all();
    $single_record = FALSE;
    // @todo: also check for sub-calls that may have an identifier in the filter
    // but return multiple results (ex. /seedlots/{seedLotDbId}/transactions).
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
      $page_size = $this->getCleanPageSize(
        $config,
        $request->query->get('pageSize')
      );
      if (!empty($page_size)) {
        $filters['#pageSize'] = $page_size;
      }
    }

    // Get associated data type.
    $mapping_loader = \Drupal::service('entity_type.manager')->getStorage('brapidatatype');
    $datatype = array_keys($brapi_def['calls'][$call]['data_types'])[0];
    // Special case for list of lists.
    if ('ListTypes' == $datatype) {
      $datatype = 'ListSummary';
    }
    $datatype_id = brapi_generate_datatype_id($datatype, $version, $active_def);
    $datatype_mapping = $mapping_loader->load($datatype_id);
    if (empty($datatype_mapping)) {
      $message = "No mapping available for data type '$datatype_id'.";
      \Drupal::logger('brapi')->error($message);
      throw new NotFoundHttpException($message);
    }
    // @todo: manage PUT (and POST?) calls and record data.
    if ('put' == $method) {
      $message = "Not implemented yet.";
      \Drupal::logger('brapi')->error($message);
      \Drupal::messenger()->addWarning($message);
    }
    // Manage old /v1/*-search calls and /v*/search/* calls.
    if (str_contains($call, 'search')) {
      $json_input = $this->getPostData($request);
      // Get lower key filter to manage invalid capitalization.
      $json_low_keys = array_combine(
        array_map('strtolower', array_keys($json_input ?? [])),
        array_keys($json_input ?? [])
      );
      $datatype = array_keys($brapi_def['calls'][$call]['data_types'])[0];
      $fields = array_keys($brapi_def['data_types'][$datatype]['fields']);
      $referenced_datatypes = [];
      // Process simple field filters.
      foreach ($fields as $field_name) {
        // Also keep track of referenced datatypes.
        if (brapi_is_reference_to_datatype($field_name, $datatype, $brapi_def)) {
          $referenced_datatypes[$field_name] = [];
        }
        // Try from GET parameters.
        $param_value =
          $request->query->get($field_name )
          ?? $request->query->get(brapi_get_term_plural($field_name))
          ?? $request->query->get(brapi_get_term_singular($field_name))
        ;
        // If nothing from GET, try from POST parameters.
        if ((!isset($param_value)) || ('' == $param_value)) {
          if (array_key_exists($field_name, $json_input)) {
            $param_value = $json_input[$field_name];
            unset($json_input[$field_name]);
          }
          elseif (
            array_key_exists(
              $plural_field_name = brapi_get_term_plural($field_name),
              $json_input
            )
          ) {
            // Check for plural and array of values.
            $param_value = $json_input[$plural_field_name];
            unset($json_input[$plural_field_name]);
          }
          elseif (
            array_key_exists(
              $singular_field_name = brapi_get_term_singular($field_name),
              $json_input
            )
          ) {
            // Check for plural and array of values.
            $param_value = $json_input[$singular_field_name];
            unset($json_input[$singular_field_name]);
          }
          elseif (
            array_key_exists(
              $field_name_low = strtolower($field_name),
              $json_low_keys
            )
          ) {
            // Also check for invalid capitalization.
            $invalid_case_field = $json_low_keys[$field_name_low];
            $param_value = $json_input[$invalid_case_field];
            unset($json_input[$invalid_case_field]);
            // Warn.
            $status[] = [
              'message'     => 'Invalid capitalization for post filters adjusted: "' . $invalid_case_field . '" should be "' . $field_name . '"',
              'messageType' => 'WARNING',
            ];
          }
        }
        if (isset($param_value) && ('' != $param_value)) {
          // We got something, set the filter.
          $filters[$field_name] = $param_value;
        }
      }

      // Process filters on sub-objects.
      // Loop on remaining unresolved filters.
      foreach ($json_input as $filter_field => $filter_value) {
        // Loop on fields that are references to other datatypes.
        foreach (array_keys($referenced_datatypes) as $ref_datatype) {
          // Check if the filter corresponds to a referenced object field.
          if (0 === strncmp(
              $filter_field,
              $ref_datatype,
              strlen($ref_datatype)
            )
          ) {
            // Add filter to referenced object filters.
            // Remove subfield datatype name used as prefix, singularize and
            // lowercase first char for camelCase.
            $ref_field = lcfirst(
              brapi_get_term_singular(
                substr($filter_field, strlen($ref_datatype))
              )
            );
            $referenced_datatypes[$ref_datatype][$ref_field] = $filter_value;
            unset($json_input[$filter_field]);
          }
        }
      }
      // Now "convert" referenced filter.
      foreach ($referenced_datatypes as $ref_datatype => $ref_filters) {
        // Find corresponding identifiers and add them as filter for current.
        if (!empty($ref_filters)) {
          $brapi_submapping = brapi_get_referenced_datatype_mapping(
            $ref_datatype,
            $datatype_mapping,
            $brapi_def
          );
          if ($brapi_submapping) {
            $sub_entities = $brapi_submapping->getBrapiData($ref_filters);
            // Get sub-object ID field name.
            $subdatatype_id_field = brapi_get_datatype_identifier($brapi_submapping->getBrapiDatatype(), $brapi_def);
            // Set $filter to fill the appropriate key of $filters.
            if (array_key_exists(lcfirst($ref_datatype), $brapi_def['data_types'][$datatype]['fields'])) {
              $filter = &$filters[lcfirst($ref_datatype)];
            }
            elseif (array_key_exists($subdatatype_id_field, $brapi_def['data_types'][$datatype]['fields'])) {
              $filter = &$filters[$subdatatype_id_field];
            }
            else {
              $status[] = [
                'message'     => 'No suitable field found to filter reference to "' . $ref_datatype . '" for ' . $datatype . '. Related filters ignored.',
                'messageType' => 'WARNING',
              ];
              continue 1;
            }
            // Initialize $filter with an empty array to be filled.
            $filter = [];
            if (empty($sub_entities['entities'])) {
              // No match, use an unexisting identifier.
              // We assume '-1' is never used as an identifier in databases.
              $filter[] = '-1';
            }
            else {
              // Add each sub-object identifier to the filter.
              foreach (($sub_entities['entities'] ?? []) as $sub_entity) {
                $filter[] = $sub_entity[$subdatatype_id_field];
              }
            }
          }
          else {
            $status[] = [
              'message'     => 'No "' . $ref_datatype . '" submapping available for ' . $datatype . '. Related filters ignored.',
              'messageType' => 'WARNING',
            ];
          }
        }
      }

      // @todo: Process sub/complex fields (GET and POST).
      // ex. Germplasm externalReferences
      // Loop on remaining unresolved filters.
      // foreach ($json_input as $filter_field => $filter_value) {
      // ...
      // }

      // @todo: Process non-trivial field filters (GET and POST).
      // ex.: Trait GET uses filter observationVariableDbIds.
      //      We could check if it ends by DbId or DbIds and see if the prefix
      //      is a datatype that has a traitDbId field or trait as a subobject.
      // Loop on remaining unresolved filters.
      // foreach ($json_input as $filter_field => $filter_value) {
      // ...
      // }

      // Report unprocessed filters.
      if (!empty($json_input)) {
        $status[] = [
          'message'     => 'Unsupported filter(s): ' . implode(', ', array_keys($json_input)),
          'messageType' => 'WARNING',
        ];
      }
    }
    // Process query filters.
    $processed_filters = ['page', 'pageSize', 'Authorization', ];
    foreach ($brapi_def['calls'][$call]['definition'][$method]['parameters'] as $filter_param) {
      $param_name = $filter_param['name'];
      // Only take into account parameters in URL query string and skip
      // output control parameters.
      if (('query' != $filter_param['in'])
          || (in_array($param_name, ['page', 'pageSize', 'Authorization', ]))
      ) {
        continue 1;
      }
      $param_value = $request->query->get($param_name);
      if ((NULL != $param_value) && ('' != $param_value)) {
        $filters[$param_name] = $param_value;
        $processed_filters[] = $param_name;
      }
    }
    // Keep track of unprocessed filters in metadata as warning.
    $unsupported_filters = array_diff(
      $request->query->keys(),
      $processed_filters
    );
    if (!empty($unsupported_filters)) {
      $status[] = [
        'message'     => 'Unsupported query filters: ' . implode(', ', $unsupported_filters),
        'messageType' => 'WARNING',
      ];
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
      $result = ['result' => current($entities)];
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
      'status'      =>  $status ?: NULL,
    ];

    $metadata = $this->generateMetadata($request, $config, $parameters);
    return $metadata + $result;
  }

  /**
   *
   */
  public function getBrapiData(
    $datatype_mapping,
    $filters
  ) {
    // Fetch BrAPI object(s).
    $brapi_data = $datatype_mapping->getBrapiData($filters);
    $entities = $brapi_data['entities'];

    return $entities;
  }

}
