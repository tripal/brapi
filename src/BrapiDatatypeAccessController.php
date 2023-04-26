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
    // @todo: manage access permission BRAPI_PERMISSION_USE, BRAPI_PERMISSION_SPECIFIC and BRAPI_PERMISSION_EDIT.
    // 'view', 'update' or 'delete'
    // if ($operation == 'view') {
    //   return AccessResult::allowed();
    // }
    return parent::checkAccess($entity, $operation, $account);
  }

}
