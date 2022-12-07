<?php

namespace Drupal\brapi\EventSubscriber;

use Symfony\Component\HttpKernel\KernelEvents;
use Symfony\Component\HttpKernel\Event;
use Symfony\Component\HttpKernel\Event\GetResponseEvent;
use Symfony\Component\HttpKernel\Event\TerminateEvent;
use Symfony\Component\EventDispatcher\EventSubscriberInterface;

/**
 *
 */
class BrapiSubscriber implements EventSubscriberInterface {

  /**
   * {@inheritdoc}
   */
  public static function getSubscribedEvents() {
    $events[KernelEvents::REQUEST][] = ['BrapiRequest', 20];
    $events[KernelEvents::TERMINATE][] = 'BrapiTerminate';
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
   * This hook implementation uses the "bearer" token provided by
   * BrAPI-compliant clients to login clients.
   *
   * To generate a token:
   *
   *   $uid = ...; // Get user identifier.
   *   // $token_id is the user token.
   *   $token_id = bin2hex(random_bytes(16));
   *   $cid = 'brapi_token:' . $token_id;
   *   // 'mgis' should be a valid user name.
   *   $data = ['username' => 'mgis'];
   *   // The token will expire in 1 day (=86400sec).
   *   // Other possibility for permanent token: Cache::PERMANENT.
   *   $expiration = time() + 86400;
   *   \Drupal::cache('brapi_token')->set($cid, $data, $expiration, ['user:' . $uid, 'brapi']);
   *
   * @see Symfony\Component\HttpKernel\KernelEvents for details
   *
   * @param Symfony\Component\HttpKernel\Event\GetResponseEvent $event
   *   The response event to process.
   */
  public function BrapiRequest(GetResponseEvent $event) {
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
      $name = '';
      // Try to get the token.
      $cid = 'brapi:' . $bearer;
      if ($cache = \Drupal::cache('brapi_token')->get($cid)) {
        $name = $cache->data['username'];
      }

      // Make sure we got a user to login.
      if (!empty($name)) {
        // Get user account.
        $account = user_load_by_name($name);
        // Try to login user.
        if ($account) {
          user_login_finalize($account);
        }
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
   * @return string
   *   Returns the bearer content (without a "Bearer " prefix).
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
    // If still not found, tries the "apache" way.
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
    $got_bearer = preg_match(
      "/(?:^|\s)Bearer\s+([^\s;]+)/i",
      $headers,
      $matches
    );
    if ($got_bearer) {
      // Got the bearer.
      $bearer = $matches[1];
    }
    else {
      // Tries to get the bearer as a regular HTTP header (non-standard).
      $bearer = $request->headers->get('bearer');
    }

    return $bearer;
  }

  /**
   * Perform BrAPI heavy operation after client response has been sent.
   *
   * Peform a differed search if set.
   */
  public function BrapiTerminate(TerminateEvent $event) {
    // Launch asynchroneous search if needed.
    $async_search = \Drupal::Service('brapi.async_search');
    $async_search->performSearches();
  }

}

