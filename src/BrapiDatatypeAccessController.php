<?php

namespace Drupal\brapi;

use Drupal\Core\Access\AccessResult;
use Drupal\Core\Entity\EntityAccessControlHandler;
use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Session\AccountInterface;

/**
 * Defines an access controller for the BrAPI Datatype Mapping entity.
 *
 * @see \Drupal\brapi\Entity\BrapiDatatype
 */
class BrapiDatatypeAccessController extends EntityAccessControlHandler {

  /**
   * {@inheritdoc}
   */
  public function checkAccess(EntityInterface $entity, $operation, AccountInterface $account) {
    if (is_a($entity, \Drupal\brapi\Entity\BrapiDatatype::class)) {
      // Allow access to BrAPI mapping to any people with any BrAPI access.
      if ($operation == 'view'
        && ($account->hasPermission(BRAPI_PERMISSION_USE)
          || $account->hasPermission(BRAPI_PERMISSION_EDIT)
          || $account->hasPermission(BRAPI_PERMISSION_SPECIFIC))
      ) {
        return AccessResult::allowed();
      }
      // Restrict BrAPI mapping modification to admins.
      if (($operation == 'update' || $operation == 'delete')
          && ($account->hasPermission(BRAPI_PERMISSION_ADMIN)
            || $account->hasPermission('administer site configuration'))
      ) {
        return AccessResult::allowed();
      }
    }
    return parent::checkAccess($entity, $operation, $account);
  }

}
