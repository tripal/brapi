<?php

namespace Drupal\brapi\EventSubscriber;

use Drupal\brapi\Entity\BrapiToken;
use Symfony\Component\HttpKernel\KernelEvents;
use Symfony\Component\HttpKernel\Event;
use Symfony\Component\HttpKernel\Event\RequestEvent;
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
   * @see \Symfony\Component\HttpKernel\KernelEvents
   * @see \Drupal\brapi\Entity\BrapiToken
   *
   * @param Symfony\Component\HttpKernel\Event\RequestEvent $event
   *   The response event to process.
   */
  public function BrapiRequest(RequestEvent $event) {
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
    $config = \Drupal::config('brapi.settings');
    if (($request->isSecure() || $config->get('insecure'))
        && !empty($bearer)
    ) {
      $name = '';
      // Try to get the token.
      $tokens = \Drupal::entityTypeManager()
        ->getStorage('brapi_token')
        ->loadByProperties(['token' => $bearer])
      ;
      if (count($tokens)) {
        $token = current($tokens);
        $user_id = $token->user_id->getValue()[0]['target_id'] ?? 0;
        // Make sure we got a user to login.
        if ($user_id) {
          $account = \Drupal\user\Entity\User::load($user_id);
        }

        // Try to login user.
        if ($account) {
          user_login_finalize($account);
        }
      }

    }
    elseif (!empty($bearer) && !$config->get('insecure')) {
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

