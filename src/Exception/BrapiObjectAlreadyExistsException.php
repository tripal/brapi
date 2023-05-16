<?php

namespace Drupal\brapi\Exception;

use Drupal\brapi\Exception\BrapiObjectException;

/**
 * Exception thrown when a new BrAPI object needs to be created with a given
 * identifier but another BrAPI object with the same identifier already exists.
 */
class BrapiObjectAlreadyExistsException extends BrapiObjectException {}
