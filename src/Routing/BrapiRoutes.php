<?php

namespace Drupal\brapi\Routing;

use Symfony\Component\Routing\Route;
use Symfony\Component\Routing\RouteCollection;

/**
 * Defines dynamic routes.
 */
class BrapiRoutes {

  /**
   * {@inheritdoc}
   */
  public function routes() {

    // Get current settings.
    $config = \Drupal::config('brapi.settings');
    $version_settings = $config->get('calls') ?? [];

    // Set available call routes.
    $route_collection = new RouteCollection();
    foreach ($version_settings as $version => $calls) {
      foreach ($calls as $call => $call_settings) {
        $route = new Route(
          '/brapi/' . $version . $call,
          [
            '_controller' => '\Drupal\brapi\Controller\BrapiController::brapiCall',
            '_title' => 'BrAPI Call',
          ],
          // BrAPI accesses are managed by BrAPI controller as the route
          // permissions are checked before BrAPI gets a change to login a user
          //  through his/her bearer.
          ['_access' => 'TRUE',]
        );
        // (Invalid) methods are managed by BrAPI controller.
        $route->setMethods(['GET', 'POST', 'PUT', 'DELETE']);
        $route_name =
          'brapi.'
          . $version
          . strtolower(preg_replace('/\W/', '_', $call))
        ;
        $route_collection->add($route_name, $route);
      }
    }
    // Add support for invalid routes.
    foreach (['v1', 'v2'] as $version) {
      $levels = [
        'brapi.' . $version . '_invalid'   => '',
        'brapi.' . $version . '_invalid_1' => '/{level1}',
        'brapi.' . $version . '_invalid_2' => '/{level1}/{level2}',
        'brapi.' . $version . '_invalid_3' => '/{level1}/{level2}/{level3}',
        'brapi.' . $version . '_invalid_4' => '/{level1}/{level2}/{level3}/{level4}',
      ];
      foreach ($levels as $route_name => $sub_route) {
        $route = new Route(
          '/brapi/' . $version . $sub_route,
          [
            '_controller' => '\Drupal\brapi\Controller\BrapiController::brapiInvalidCall',
            '_title' => 'BrAPI Call',
          ],
          ['_access' => 'TRUE',]
        );
        $route->setMethods(['GET', 'POST', 'PUT', 'DELETE']);
        $route_collection->add($route_name, $route);
      }
    }
    return $route_collection;
  }

}
