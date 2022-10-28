<?php

namespace Drupal\brapi\EventSubscriber;

use Symfony\Component\HttpKernel\KernelEvents;
use Symfony\Component\HttpKernel\Event;
use Symfony\Component\HttpKernel\Event\GetResponseEvent;
use Symfony\Component\EventDispatcher\EventSubscriberInterface;

/**
 * Subscribe to IncidentEvents::NEW_REPORT events and react to new reports.
 *
 * In this example we subscribe to all IncidentEvents::NEW_REPORT events and
 * point to two different methods to execute when the event is triggered. In
 * each method we have some custom logic that determines if we want to react to
 * the event by examining the event object, and the displaying a message to the
 * user indicating whether or not that method reacted to the event.
 *
 * By convention, classes subscribing to an event live in the
 * Drupal/{module_name}/EventSubscriber namespace.
 *
 * @ingroup events_example
 */
class BrapiSubscriber implements EventSubscriberInterface {

  /**
   * {@inheritdoc}
   */
  public static function getSubscribedEvents() {
    $events[KernelEvents::REQUEST][] = array('BrapiLoad', 20);
    return $events;
  }

  /**
   * Manages the BrAPI login.
   *
   * Used for KernelEvents::REQUEST event.
   *
   * BrAPI-compliant clients are not supposed to support cookies but rather
   * support "bearer" token instead. Drupal does not work with bearer token and
   * uses session cookies.
   * This hook implementation is used to convert the "bearer" token provided by
   * BrAPI-compliant clients into a Drupal session cookie.
   * The initial bearer token is provided by this BrAPI implementation when the
   * client uses the login service (POST /brapi/v1/token). The bearer token is
   * generated using Drupal login system that creates a session object. The
   * session cookie is serialized in a string ("name=id") and provided as token
   * to the client application that will use it for the next calls that
   * requires authentication.
   *
   * BrAPI-compliant clients provide this kind of bearer token in the HTTP header:
   * Authorization: Bearer SESS13cd44e3aa3714d0cc373e81c4e33f5b=JoAO7C2aGrSkteEtoy
   *
   * The bearer value is break into 2 pieces that correspond to the session name
   * and the session ID.
   * 
   * @see Symfony\Component\HttpKernel\KernelEvents for details
   *
   * @param Symfony\Component\HttpKernel\Event\GetResponseEvent $event
   *   The Event to process.
   */
  public function BrapiLoad(GetResponseEvent $event) {
    $request = \Drupal::request();
    $route = $request->attributes->get('_route_object');
    
    // Skip non-BrAPI calls.
    if (!preg_match('#^/brapi/(v\d)(/.+)#', $route->getPath(), $matches)) {
      return;
    }
    $version = $matches[1];
    $call = $matches[2];
    
    // Tries to get a bearer (ie. the authorization token).
    $bearer = $this->getBearer();

    // Only allow authentication through HTTPS chanel.
    // Get session id from bearer token (HTTP header provided by the client).
    if ($request->isSecure() && !empty($bearer)) {
      $session = $request->getSession();
      // Parses bearer to extract session name and ID.
      $got_authentication = preg_match(
        "/^([^\s=]+)=(\S+)$/i",
        $bearer,
        $matches
      );
      if ($got_authentication) {
        // $session_bag = $session->getBag();
        // Update client cookies.
        $request->cookies->set('cookie-agreed-version', '1.0.0');
        $request->cookies->set('cookie-agreed', '1');
        $request->cookies->set($matches[1], $matches[2]);
        // Set HTTP header cookies for others from the bearer token.
        // $http_cookie = $request->headers->get('COOKIE');
        // if ($http_cookie) {
        //   $cookie_already_set =
        //       (FALSE !== strpos($http_cookie, $matches[1]));
        // }
        // 
        // if (empty($cookie_already_set)) {
          $request->headers->set(
            'COOKIE',
            "cookie-agreed-version=1.0.0; cookie-agreed=1; " . $matches[1] . "=" . $matches[2]
            //$http_cookie . "; " . $matches[1] . "=" . $matches[2]
          );
        //}

        // Update PHP session info.
        // $session->setName($matches[1]);
        $session->setId($matches[2]);
        // Restart session system with new infos.
        $session->migrate();
        // // Restore previous session data.
        // if (!empty($session_bag)) {
        //   $session->registerBag($session_bag)
        // }
      }
    }
    elseif (!empty($bearer)) {
      \Drupal::messenger()->addMessage('BrAPI: Authentication is only supported through HTTPS (secure http)!', \Drupal\Core\Messenger\MessengerInterface::TYPE_WARNING);
      \Drupal::logger('brapi')->warning('A user tried to use a BrAPI token without secure HTTPS connection.');
    }
  }

  /** 
   * Get HTTP Bearer.
   *
   */
  function getBearer() {
    $request = \Drupal::request();
    // Tries to get the HTTP authorization header in different ways.
    $headers = trim(
      $request->server->get('Authorization')
      ?? $request->server->get('HTTP_AUTHORIZATION')
      ?? $request->server->get('AUTHORIZATION')
      ?? ''
    );
    // If not found, tries the "apache" way.
    if (empty($headers) && function_exists('apache_request_headers')) {
      $request_headers = apache_request_headers();
      // Server-side fix for bug in old Android versions (a nice side-effect of
      // this fix means we don't care about capitalization for Authorization).
      $request_headers = array_combine(
        array_map('ucwords', array_keys($request_headers)),
        array_values($request_headers)
      );
      if (isset($request_headers['Authorization'])) {
        $headers = trim($request_headers['Authorization']);
      }
    }
    // Tries to parse the bearer.
    $got_authentication = preg_match(
      "/\s*Bearer\s+([^\s=]+=\S+)/i",
      $headers,
      $matches
    );
    if ($got_authentication) {
      // Got the bearer with the expected format.
      $bearer = $matches[1];
    }
    else {
      // Tries to get the bearer as a regular HTTP header (non-standard).
      $bearer = $request->headers->get('bearer');
    }

    return $bearer;
  }

}

