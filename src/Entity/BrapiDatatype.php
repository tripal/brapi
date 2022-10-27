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
 *     "contentFieldPath",
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
   *   An array with "total_count" and "entities" as an array of BrAPI entity.
   */
  public function getBrapiData(array $parameters) :array {
    
    $storage = \Drupal::service('entity_type.manager')->getStorage($this->getMappedEntityTypeId());
    if (empty($storage)) {
      \Drupal::logger('brapi')->error("No storage available for content type '" . $this->contentType . "'.");
      return [];
    }

    $filters = [];
    foreach ($parameters as $parameter => $value) {
      $field_name = $this->getDrupalMappedField($parameter);
      if (!empty($field_name)) {
        $filters[$field_name] = $value;
      }
    }
    $query = $storage->getQuery();
    $count_query = $storage->getQuery()->count();
    // We manage access permission on BrAPI entities.
    // @todo: document with restricted content would be accessible if mapped to BrAPI entities.
    $query->accessCheck(FALSE);
    $count_query->accessCheck(FALSE);
    foreach ($filters as $name => $value) {
      $query->condition($name, (array) $value, 'IN');
      $count_query->condition($name, (array) $value, 'IN');
    }
    $query->range(
      empty($parameters['#page']) ? 0 : $parameters['#page'],
      empty($parameters['#pageSize']) ? BRAPI_DEFAULT_PAGE_SIZE : $parameters['#pageSize']
    );
    $item_count = intval($count_query->execute());
    $ids = $query->execute();

    $entities = $storage->loadMultiple($ids);
    $result = [];
    foreach ($entities as $id => $entity) {
      $brapi_data = [];
      foreach ($this->mapping as $brapi_field => $drupal_field) {
        if (!empty($drupal_field)) {
          //@todo: Check for entity reference and sub-mapping.
          /*
  -un champ string normal vers un champ d'une entity référencée (ex.: collection name)
    on détecte l'external entity sur le formulaire de mapping et on ajoute en
    ajax le choix des champs de l'entité référencée.
    collection.field (BrAPI) => field_institute (champ Drupal vers entity ref "collection")
    collection.mapping (BrAPI) => field_institute_name (champ Drupal de "collection")
  -un champ "object" (ou plusieurs) vers une entity référencée convertie en array (ex. additionalInfo)
    mapping normal, pas de "-mapping" donc utilisation de "toArray()".
    additionalInfo => field_infos (entiy ref.)
  -un champ "object" (ou plusieurs) vers un champ classique (ex. additionalInfo)
    ajout de cases à sélectionner "pas de parsing", "parser comme du JSON"
    additionalInfo.field => field_infos (string)
    additionalInfo.parse => 'JSON'
  -une liste d'objets typés BrAPI à mapper vers des entities référencées déjà mappés (ex. donors)
    on détecte l'external entity sur le formulaire de mapping et on ajoute en
    ajax le choix des champs de l'entité référencée.
    donors.field (BrAPI) => field_donors (entity ref. vers "Donors", mappé pour Germplasm_donors)
    donors.mapping => id (récupéré par le champ texte)
    donors.brapi   => v2-2.0-Germplasm_donors (récupéré automatiquement)
  -un object à partir des valeurs de l'objet en cours (ex. geo-location, storageTypes)
    on ajoute un mapping v2-2.0-Germplasm-storageTypes
    storageTypes.brapi => v2-2.0-Germplasm-storageTypes

          */
          try {
            // Check if there is a sub-mapping.
            if (is_array($drupal_field)) {
              // @todo: get BrapiData from referenced entity mapping if one...
              // $brapi_datatype contains the BrAPI datatype sub-mapping machine name.
              // if $drupal_ref_field is empy, it means the sub-mapping uses current Drupal entity ID.
              // if $drupal_ref_field is not empty, it contains the field name containing the entity ID to use.
              // -Load the corresponding BrAPI datatype sub-mapping from $brapi_datatype.
              // -call getBrapiData
              $brapi_data[$brapi_field] = [];
            }
            else {
              $field = $entity->get($drupal_field);
              if ($field->getFieldDefinition()->getType() == 'entity_reference') {
                // We got referenced entities.
                $brapi_data[$brapi_field] = [];
                // Add all of them as arrays.
                $brapi_data[$brapi_field] = array_map(
                  function ($ref_entity) { return $ref_entity->toArray(); },
                  $field->referencedEntities()
                );
              }
              else {
                // Regular field, get it as string.
                $brapi_data[$brapi_field] = $field->getString();
              }
            }
          }
          catch (InvalidArgumentException $e) {
            // @todo: warn for invalid mapping.
          }
        }
        else {
          $brapi_data[$brapi_field] = NULL;
        }
      }
      $result[] = $brapi_data;
    }
    return ['total_count' => $item_count, 'entities' => $result];
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
   * The ID ([v1|v2]-[BrAPI full version]-[BrAPI datatype name]).
   *
   * In case of sub-mapping, [BrAPI datatype name] can be followed by one or
   * more "-[BrAPI field name]".
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
   * The associated Drupal content type machine name ([type]:[bundle]).
   *
   * @var string
   */
  public $contentType;

  /**
   * For sub-mapping, contains a list of field machine names (separator ':').
   *
   * @var string
   */
  public $contentFieldPath;

  /**
   * The field mapping.
   *
   * @var array
   */
  public $mapping;

}
