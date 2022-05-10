<?php

namespace Drupal\brapi\Entity;

use Drupal\Core\Config\Entity\ConfigEntityBase;

/**
 * Defines BrAPI Datatype Mapping entity.
 *
 * @ConfigEntityType(
 *   id = "brapidatatype",
 *   label = @Translation("BrAPI Datatype Mapping"),
 *   admin_permission = "administer site configuration",
 *   handlers = {
 *     "access" = "Drupal\brapi\BrapiDatatypeAccessController",
 *     "list_builder" = "Drupal\brapi\Controller\BrapiDatatypeListBuilder",
 *     "form" = {
 *       "add" = "Drupal\brapi\Form\BrapiDatatypeAddForm",
 *       "edit" = "Drupal\brapi\Form\BrapiDatatypeEditForm",
 *       "delete" = "Drupal\brapi\Form\BrapiDatatypeDeleteForm"
 *     }
 *   },
 *   entity_keys = {
 *     "id" = "id",
 *     "label" = "label"
 *   },
 *   links = {
 *     "edit-form" = "/brapi/admin/datatypes/manage/{brapidatatype}",
 *     "delete-form" = "/brapi/admin/datatypes/manage/{brapidatatype}/delete"
 *   },
 *   config_export = {
 *     "id",
 *     "uuid",
 *     "label",
 *     "contentType",
 *     "mapping"
 *   }
 * )
 */
class BrapiDatatype extends ConfigEntityBase {

  /**
   * The ID.
   *
   * @var string
   */
  public $id;

  /**
   * The UUID.
   *
   * @var string
   */
  public $uuid;

  /**
   * The label.
   *
   * @var string
   */
  public $label;

  /**
   * The associated content type machine name.
   *
   * @var string
   */
  public $contentType;

  /**
   * The field mapping.
   *
   * @var array
   */
  public $mapping;

}
