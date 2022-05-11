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
   * Returns associated Drupal content type machine name.
   *
   * @return string
   *   The Drupal content type machine name.
   */
  public function getMappedEntityTypeId() :string {
    return substr($this->contentType, 0, strpos($this->contentType, ':'));
  }

  /**
   * Returns associated Drupal content type machine name.
   *
   * @return string
   *   The Drupal content type machine name.
   */
  public function getMappedEntityBundleId() :string {
    return substr($this->contentType, strpos($this->contentType, ':') + 1);
  }

  /**
   * Loads a BrAPI entity according to parameters and current mapping.
   *
   * @param array $parameters
   *   An associative array. Keys starting with '#' contain special values such
   *   as pager data and other keys are considered as field names with their
   *   associated values to use to filter data.
   *
   * @return array
   *   A BrAPI entity data in an associative array.
   */
  public function getBrapiData(array $parameters) :array {
    
    $storage = \Drupal::service('entity_type.manager')->getStorage($this->getMappedEntityTypeId());
    if (empty($storage)) {
      \Drupal::logger('brapi')->error("No storage available for content type '" . $this->contentType . "'.");
      return [];
    }
    // $brapi_def = brapi_get_definition($this->, $this->);
    
    $filters = [];
    foreach ($parameters as $parameter => $value) {
      $field_name = $this->getDrupalMappedField($parameter);
      if (!empty($field_name)) {
        $filters[$field_name] = $value;
      }
    }
    //$ids = $storage->loadByProperties($filters);
    $query = $storage->getQuery();
    // We manage access permission on BrAPI entities.
    // @todo: document that restricted content would be accessible if mapped to BrAPI entities.
    $query->accessCheck(FALSE);
    foreach ($filters as $name => $value) {
      $query->condition($name, (array) $value, 'IN');
    }
    $query->range(
      empty($parameters['#page']) ? 0 : $parameters['#page'],
      empty($parameters['#pageSize']) ? BRAPI_DEFAULT_PAGE_SIZE : $parameters['#pageSize']
    );
    $ids = $query->execute();

    $result = [];
    foreach ($ids as $id) {
      // @todo: use loadMultiple().
      $entity = $storage->load($id);
      if (empty($entity)) {
        \Drupal::logger('brapi')->error("Failed to load content id '$id' (content type '" . $this->contentType . "').");
        continue 1;
      }
      $entity_data = $entity->toArray();
      $brapi_data = [];
      foreach ($this->mapping as $brapi_field => $drupal_field) {
        if (!empty($drupal_field)) {
          // @todo: get field value (no "[0]['value']" hard coding).
          $brapi_data[$brapi_field] = $entity_data[$drupal_field][0]['value'];
        }
        else {
          $brapi_data[$brapi_field] = NULL;
        }
      }
      $result[] = $brapi_data;
    }
    return $result;
  }

  /**
   * Returns the related Drupal content field name.
   *
   * Returns the associated Drupal content field name of a given BrAPI field
   * name.
   *
   * @param string $brapi_field_name
   *   A BrAPI field name.
   *
   * @return string
   *   The associated Drupal field name or an empty string if no mapping was
   *   set.
   */
  public function getDrupalMappedField(string $brapi_field_name) :string {
    return $this->mapping[$brapi_field_name] ?? '';
  }

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
