<?php

namespace Drupal\brapi\Controller;

use Drupal\Core\Config\Entity\ConfigEntityListBuilder;
use Drupal\Core\Entity\EntityInterface;

/**
 * Provides a listing of BrAPI Datatype Mapping entities.
 */
class BrapiDatatypeListBuilder extends ConfigEntityListBuilder {

  /**
   * {@inheritdoc}
   */
  protected function getModuleName() {
    return 'brapi';
  }

  /**
   * {@inheritdoc}
   */
  public function buildHeader() {
    $header['label'] = $this->t('BrAPI Datatype Mapping');
    $header['machine_name'] = $this->t('Machine Name');
    // $header['mapping'] = $this->t('Mapping');
    return $header + parent::buildHeader();
  }

  /**
   * {@inheritdoc}
   */
  public function buildRow(EntityInterface $entity) {
    $row['label'] = $entity->label();
    $row['machine_name'] = $entity->id();
    // $row['mapping'] = print_r($entity->mapping, TRUE);
    return $row + parent::buildRow($entity);
  }

}
