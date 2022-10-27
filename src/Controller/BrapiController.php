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

class BrapiController extends ControllerBase {

  /**
   * Initial landing page explaining the use of the module.
   *
   * We create a render array and specify the theme to be used through the use
   * of #theme_wrappers. With all output, we aim to leave the content as a
   * render array just as long as possible, so that other modules (or the theme)
   * can alter it.
   *
   * @see render_example.module
   * @see form_example_elements.inc
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
   * Export test as JSON.
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
    $variables = $request->attributes->get('_raw_variables')->all();
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

    // Get optional POST JSON data.
    $content = $request->getContent();
    if (!empty($content)) {
      $json_input = json_decode($content, TRUE);
    }

    $single_datatype = FALSE;
    if (1 == count($brapi_def['calls'][$call]['data_types'])) {
      $total_count = 1;
      $total_pages = 1;
      if (!empty($variables)) {
        $single_datatype = TRUE;
        $page = 0;
        $page_size = 1;
      }
      else {
        $page = $request->query->get('page') ?? 0;
        if (!empty($page)) {
          $variables['#page'] = $page;
        }
        $page_size = $request->query->get('pageSize') ?? BRAPI_DEFAULT_PAGE_SIZE;
        if (!empty($page_size)) {
          $variables['#pageSize'] = $page_size;
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
      // $brapi_def['calls'][$call]['definition'][$method]['parameters']
      foreach ($brapi_def['calls'][$call]['definition'][$method]['parameters'] as $filter_param) {
        // Only take into account parameters in URL query string and skip
        // output control parameters.
        if (('query' != $filter_param['in'])
            || (in_array($param_name, ['page', 'pageSize', 'Authorization']))
        ) {
          continue 1;
        }
        $param_name = $filter_param['name'];
        $param_value = $request->query->get($param_name);
        if ((NULL != $param_value) && ($param_value != '')) {
          $variables[$param_name] = $param_value;
        }
      }
      $brapi_data = $datatype_mapping->getBrapiData($variables);
      $entities = $brapi_data['entities'];
      // Update $total_count and $total_pages.
      $total_count = $brapi_data['total_count'];
      $total_pages = ceil($total_count/$page_size);
    }
    elseif (('v2' == $version) && ('/serverinfo' == $call)) {
      // v2 /serverinfo call.
      $calls = [];
      foreach ($call_settings[$version] as $call => $methods) {
        $methods = array_map('strtoupper', array_keys(array_filter($methods)));
        $calls[] = [
          'contentTypes' => [BRAPI_MIME_JSON],
          'dataTypes'    => [BRAPI_MIME_JSON],
          'methods'      => $methods,
          'service'      => substr($call, 1),
          'versions'     => [$active_def],
        ];
      }
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
      $page_size = 1;
      $page = 0;
      $total_count = 1;
      $total_pages = 1;
    }
    elseif (('v1' == $version) && ('/calls' == $call)) {
      // v1 /calls call.
      $calls = [];
      foreach ($call_settings[$version] as $call => $methods) {
        $methods = array_map('strtoupper', array_keys(array_filter($methods)));
        $calls[] = [
          'call'         => substr($call, 1),
          'dataTypes'    => [BRAPI_MIME_JSON],
          'datatypes'    => [BRAPI_MIME_JSON],
          'methods'      => $methods,
          'versions'     => [$active_def],
        ];
      }
      $result = ['result' => ['data' => $calls]];
      $page_size = 1;
      $page = 0;
      $total_count = 1;
      $total_pages = 1;
    }
    elseif (('v1' == $version) && ('/login' == $call)) {
      // Enforce HTTPS use.
      if (!$request->isSecure()) {
        throw new \Symfony\Component\HttpKernel\Exception\PreconditionFailedHttpException(
          'You must use HTTPS protocol to login. Login through unsecure HTTP is forbidden.'
        );
      }
      
      $page_size = 1;
      $page = 0;
      $total_count = 1;
      $total_pages = 1;

      // Check for log in request.
      if ('post' == $method) {
        // Get user log in data.
        $password = trim($json_input['password']) ?? '';
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
            throw new \Symfony\Component\HttpKernel\Exception\TooManyRequestsHttpException('Too many attempts.');
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
              throw new \Symfony\Component\HttpKernel\Exception\TooManyRequestsHttpException('Too many attempts.');
            }
          }

          // We are not limited by flood control, so try to authenticate.
          $uid = \Drupal::service('user.auth')->authenticate($name, $password);
        }
        if ($uid && $account) {
          user_login_finalize($account);
        }
        else {
          user_logout();
        }
      }

      // Get expiration time.
      $maxlifetime = ini_get("session.gc_maxlifetime");
      
      // Get user info.
      $user =
        User::load(\Drupal::currentUser()->id())
        ?? User::load(0)
      ;
      $account_name = $user->getAccountName();
      $display_name = $user->getDisplayName();

      $result = [
        'access_token'    => session_name() . '=' . session_id(),
        'expires_in'      => $maxlifetime,
        'userDisplayName' => $display_name,
        'client_id'       => $account_name,
      ];
    }
    elseif (('v1' == $version) && ('/logout' == $call)) {
      user_logout();
      $json_array = [];
    }
    else {
      $this->logger('brapi')->warning('Unsupported call type: %call (%version)', ['%call' => $call, '%version' => $version, ]);
    }

    // @todo: Manage call cases.
    // -special calls (like v1/calls or v2/serverinfo, login/logout)
    
    // -search calls
    // -manage special output formats (eg. /phenotypes-search/csv)
    // -check methods for the call
    //   -get: get array of parameters for GET
    //   -post: get array of parameters for POST

    // https://api.drupal.org/api/drupal/vendor!symfony!http-foundation!Request.php/class/Request/9.3.x
    // https://api.drupal.org/api/drupal/vendor%21symfony%21routing%21Route.php/class/Route/9.3.x
    // https://api.drupal.org/api/drupal/vendor%21symfony%21http-foundation%21ParameterBag.php/class/ParameterBag/9.3.x
    // POST values:
    // $value = $request->request->get('param');
    // GET values:
    // $value = $request->query->get('param');

    // Check if $result has already been set by a special call.
    if (!isset($result)) {
      // $result not set: regular data call.
      if ($single_datatype) {
        $result = ['result' => ['data' => $entities[0]]];
      }
      else {
        $result = ['result' => ['data' => $entities]];
      }
    }

    // Check if the whole response is not specific and has not been set already.
    if (!isset($json_array)) {
      $json_array = [
        'metadata' => [
            'status' => [],
            'pagination' => [
                'pageSize' => $page_size,
                'currentPage' => $page,
                'totalCount' => $total_count,
                'totalPages' => $total_pages,
            ],
            'datafiles' => [],
        ],
        // 'version' => $active_def,
        // 'call'   => $call,
        // 'method' => $method ?? 'not set',
        // 'x' => print_r($route, TRUE),
      ]
      + $result;
    }
    return new JsonResponse($json_array);
  }

}
