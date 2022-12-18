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
    $version_settings = $config->get('calls');

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
        $route->setMethods(array_map('strtoupper', array_keys($call_settings)));
        $route_name =
          'brapi.'
          . $version
          . strtolower(preg_replace('/\W/', '_', $call))
        ;
        $route_collection->add($route_name, $route);
      }
    }
    return $route_collection;
  }

}
