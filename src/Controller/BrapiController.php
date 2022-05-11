<?php

/**
 * @file
 */

namespace Drupal\brapi\Controller;

use Drupal\Core\Controller\ControllerBase;
use Drupal\Core\Url;
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
    if (empty($brapi_def)) {
      \Drupal::logger('brapi')->error("No available definition for call '$call' ($version, $active_def) using method $method.");
      throw new \Symfony\Component\HttpKernel\Exception\NotFoundHttpException();
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
        $page = $request->query->get('page');
        if (!empty($page)) {
          $variables['#page'] = $page;
        }
        $page_size = $request->query->get('pageSize');
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
      // $brapi_def['calls'][$call][$method]['parameters']
      $entities = $datatype_mapping->getBrapiData($variables);
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
    if (!empty($variables)) {
      $result = ['data' => $entities[0]];
    }
    else {
      $result = ['data' => $entities];
    }

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
      'result' => $result,
      // 'version' => $active_def,
      // 'call'   => $call,
      // 'method' => $method ?? 'not set',
      // 'x' => print_r($route, TRUE),
    ];
    return new JsonResponse($json_array);
  }

}
