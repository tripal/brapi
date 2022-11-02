<?php

namespace Drupal\brapi\Entity;

use Drupal\Core\Config\Entity\ConfigEntityBase;
use JsonPath\JsonObject;

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
   * Parse BrAPI datatype identifier into version, release, datatype and fields.
   *
   * @return array
   *  Returns BrAPI version (v1 or v2), the release supported by this datatype
   * (ex. 2.0), the datatype name and an optional array of sub-fields path on
   * wich the dataype maps.
   */
  public function parseId() {
    if (!isset($this->brapiVersion)) {
      if (preg_match(BRAPI_DATATYPE_ID_REGEXP, $this->id, $matches)) {
          $this->brapiVersion = $matches[1];
          $this->brapiRelease = $matches[2];
          $this->brapiDatatype = $matches[3];
          $this->brapiFields = array_filter(explode('-', $matches[4]));
      }
    }
    return [
      $this->brapiVersion,
      $this->brapiRelease,
      $this->brapiDatatype,
      $this->brapiFields,
    ];
  }

  /**
   * Returns associated BrAPI main version.
   *
   * @return string
   *   The BrAPI version (v1 or v2).
   */
  public function getBrapiVersion() :string {
    if (!isset($this->brapiVersion)) {
      $this->parseId();
    }
    return $this->brapiVersion;
  }

  /**
   * Returns associated BrAPI release.
   *
   * @return string
   *   The BrAPI release name (ex. 2.1).
   */
  public function getBrapiRelease() :string {
    if (!isset($this->brapiVersion)) {
      $this->parseId();
    }
    return $this->brapiRelease;
  }

  /**
   * Returns associated BrAPI datatype.
   *
   * @return string
   *   The BrAPI datatype name (ex. Germplasm).
   */
  public function getBrapiDatatype() :string {
    if (!isset($this->brapiVersion)) {
      $this->parseId();
    }
    return $this->brapiDatatype;
  }

  /**
   * Returns associated BrAPI datatype sub-field path.
   *
   * The last sub-field is the one on wich this datatype maps.
   *
   * @return array
   *   The BrAPI datatype sub-field list.
   */
  public function getBrapiFields() :array {
    if (!isset($this->brapiVersion)) {
      $this->parseId();
    }
    return $this->brapiFields;
  }

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
   * Returns associated Drupal content bundle machine name.
   *
   * @return string
   *   The Drupal content bundle machine name.
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
    
    $storage = \Drupal::service('entity_type.manager')
      ->getStorage($this->getMappedEntityTypeId())
    ;
    if (empty($storage)) {
      \Drupal::logger('brapi')->error(
        "No storage available for content type '" . $this->contentType . "'."
      );
      return [];
    }

    // Check if an entity has already been provided (for sub-field mapping).
    if (!empty($parameters['#entity'])) {
      $entities = [$parameters['#entity']->id => $parameters['#entity']];
      $item_count = 1;
    }
    else {
      // Load associated Drupal entity matching filter parameters.
      $filters = [];
      foreach ($parameters as $parameter => $value) {
        $field_name = $this->getDrupalMappedField($parameter);
        if (!empty($field_name)) {
          $filters[$field_name] = $value;
        }
      }
      $query = $storage->getQuery();
      $count_query = $storage->getQuery();
      // We manage access permission on BrAPI entities, not on Drupal associated
      // entities. A restricted Drupal entity mays be accessible if mapped to a
      // BrAPI datatype without restrictions.
      $query->accessCheck(FALSE);
      $count_query->accessCheck(FALSE);
      foreach ($filters as $name => $value) {
        $query->condition($name, (array) $value, 'IN');
        $count_query->condition($name, (array) $value, 'IN');
      }
      // Range (paggination) is not applied on the total count.
      $query->range(
        empty($parameters['#page']) ? 0 : $parameters['#page'],
        empty($parameters['#pageSize']) ? BRAPI_DEFAULT_PAGE_SIZE : $parameters['#pageSize']
      );
      // Get total number of entities matching filters.
      $item_count = intval($count_query->count()->execute());
      // Get IDs of selected entity range.
      $ids = $query->execute();
      // Load entity instances.
      $entities = $storage->loadMultiple($ids);
    }

    // Initialize result array.
    $result = [];
    // Check wich fields are arrays.
    $array_fields = [];
    list($version, $active_def, $datatype_name) = $this->parseId();
    $brapi_definition = brapi_get_definition($version, $active_def);
    $brapi_fields =
      $brapi_definition['data_types'][$datatype_name]['fields']
      ?? []
    ;
    foreach ($brapi_fields as $field_name => $field_def) {
      if (FALSE !== strrpos($field_def['type'], '[]')) {
        $array_fields[$field_name] = TRUE;
      }
    }

    // Now translate each Drupal entity into a BrAPI datatype.
    foreach ($entities as $id => $entity) {
      $brapi_data = [];
      foreach ($this->mapping as $brapi_field => $drupal_mapping) {
        if (!empty($drupal_mapping) && !empty($drupal_mapping['field'])) {
          try {
            // Check mapping type.
            if ('_brapi_datatype' == $drupal_mapping['field']) {
              // There is a sub-mapping of current entity.
              // @todo: Load BrAPI datatype $this->id . '-' . $brapi_field
              // Provide the entity $id to extract mapped BrAPI data.
              $brapi_data[$brapi_field] = [];
            }
            elseif ('_static' == $drupal_mapping['field']) {
              // Static value.
              // @todo: manage JSON data in a static string.
              $brapi_data[$brapi_field] = $drupal_mapping['static'];
            }
            else {
              // BrAPI field mapped to an entity field, get its value.
              $field = $entity->get($drupal_mapping['field']);
              // Check if field is an entity reference.
              if ($field->getFieldDefinition()->getType() == 'entity_reference') {
                // We got referenced entities.
                $brapi_data[$brapi_field] = [];
                // Check for a BrAPI-mapped entity.
                if (!empty($drupal_mapping['brapi_datatype'])) {
                  // Drupal referenced entities are BrAPI-mapped.
                  // @todo: get BrAPI mapping and load each entity.
                }
                else {
                  // Not mapped, add all referenced entities as arrays.
                  $brapi_data[$brapi_field] = array_map(
                    function ($ref_entity) { return $ref_entity->toArray(); },
                    $field->referencedEntities()
                  );
                }
                // Check if only a subfield should be returned.
                if (!empty($drupal_mapping['subfield'])) {
                  $subfield = $drupal_mapping['subfield'];
                  // Process JSONPath mapping.
                  // @see https://github.com/Galbar/JsonPath-PHP
                  $json_object = new JsonObject($brapi_data[$brapi_field]);
                  $brapi_data[$brapi_field] = $json_object->get($subfield);
                }
              }
              else {
                // Regular field, get it as string.
                $brapi_data[$brapi_field] = $field->getString();
              }
            }
          }
          catch (InvalidArgumentException $e) {
            // Warn for invalid mapping.
            \Drupal::logger('brapi')->warning(
              'Invalid datatype field mapping for BrAPI object field "%brapi_field" to Drupal entity field "%drupal_field".',
              [
                '%brapi_field'  => $drupal_field,
                '%drupal_field' => $drupal_field,
              ]
            );
          }
          
          if (!$array_fields[$brapi_field]
              && (is_array($brapi_data[$brapi_field]))
          ) {
            if (empty($brapi_data[$brapi_field])) {
              $brapi_data[$brapi_field] = NULL;
            }
            else {
              // Should not return an array, get first array value.
              $brapi_data[$brapi_field] = reset($brapi_data[$brapi_field]);
            }
          }
        }
        else {
          // No mapping for that field.
          $brapi_data[$brapi_field] = NULL;
        }
      }
      $result[] = $brapi_data;
    }
    
    // @todo: if BrAPI filtering is enabled for the call, filter.
    
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
    $mapped_field = $this->mapping[$brapi_field_name]['field'] ?? '';
    // Take into account special mappings.
    if (!empty($mapped_field) && ('_static' == $mapped_field)) {
      $mapped_field = '';
    }
    return $mapped_field;
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
   * The .
   *
   * @var string
   */
  protected $brapiVersion;

  /**
   * The .
   *
   * @var string
   */
  protected $brapiRelease;

  /**
   * The .
   *
   * @var string
   */
  protected $brapiDatatype;

  /**
   * The .
   *
   * @var array
   */
  protected $brapiFields;

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
