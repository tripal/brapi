<?php

namespace Drupal\brapi;

use Drupal\Core\Entity\ContentEntityInterface;
use Drupal\user\EntityOwnerInterface;
use Drupal\Core\Entity\EntityChangedInterface;

/**
 * Provides an interface defining a BrAPI List entity.
 */
interface BrapiListInterface extends ContentEntityInterface, EntityOwnerInterface, EntityChangedInterface {

}
