<?php

namespace Drupal\brapi\Entity;

use Drupal\brapi\Exception\BrapiObjectAlreadyExistsException;
use Drupal\brapi\Exception\BrapiObjectException;
use Drupal\brapi\Exception\BrapiStorageException;
use Drupal\Core\Config\Entity\ConfigEntityBase;
use Drupal\Core\Entity\EntityInterface;
use JsonPath\JsonObject;
use JsonPath\InvalidJsonException;
use JsonPath\InvalidJsonPathException;

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
          $this->brapiSubFields = array_filter(explode('-', $matches[4]));
      }
    }
    return [
      $this->brapiVersion,
      $this->brapiRelease,
      $this->brapiDatatype,
      $this->brapiSubFields,
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
  public function getBrapiSubFields() :array {
    if (!isset($this->brapiVersion)) {
      $this->parseId();
    }
    return $this->brapiSubFields;
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
   * Returns BrAPI data type identifier field name.
   *
   * @return string
   *   The BrAPI identifier field name.
   */
  public function getBrapiIdField() :string {
    if (!isset($this->brapiVersion)) {
      $this->parseId();
    }
    $id_field = lcfirst($this->brapiDatatype). 'DbId';
    // Make sure field exists.
    if (!array_key_exists($id_field, $this->mapping)) {
      // Field not found.
      // @todo: manage other ways to get identifier fields (invalid case, etc.).
      // @todo: manage plugin or hook for override of default behavior.
      if ('listDetailsDbId' == $id_field) {
        // Manage special case for list details.
        $id_field = 'listDbId';
      }
      else {
        $id_field = '';
      }
    }
    return $id_field;
  }

  /**
   * Loads a BrAPI entity according to parameters and current mapping.
   *
   * @param array $parameters
   *   An associative array. Keys starting with '#' contain special values such
   *   as pagination data and other keys are considered as field names with
   *   their associated values to use to filter data.
   *   Special values:
   *   #entity: used by submapping to grab field values from an already loaded
   *     entity.
   *   #include_hidden: used by submapping to also include hidden fields such as
   *     the parent object identifier (which are hidden in the returned
   *     output otherwise).
   *   #page: page number.
   *   #pageSize: page size (number of objects per page).
   *
   * @return array
   *   An array with "total_count" and "entities" as an array of BrAPI entity.
   */
  public function getBrapiData(array $parameters) :array {

    // Get data type mapping entities.
    $mapping_loader = \Drupal::service('entity_type.manager')
      ->getStorage('brapidatatype')
    ;

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
      $entities = [
        $parameters['#entity']->id() => $parameters['#entity'],
      ];
      $item_count = 1;
    }
    else {
      // Load associated Drupal entity matching filter parameters.
      $filters = [];
      $post_filters = [];
      foreach ($parameters as $parameter => $value) {
        $field_name = $this->getDrupalMappedField($parameter, TRUE);

        if (FALSE !== $field_name) {
          if ('' === $field_name) {
            // Complex mapping, use post-filtering.
            $post_filters[$parameter] = $value;
          }
          else {
            $filters[$field_name] = $value;
          }
        }
        elseif ('#' != substr($parameter, 0, 1)) {
          \Drupal::logger('brapi')->warning('Unsupported filter "' . $parameter . '" for BrAPI datatype "' . $this->getBrapiDatatype() . '"');
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
      // Manage bundle.
      $bundle_key = \Drupal::entityTypeManager()
        ->getDefinition($this->getMappedEntityTypeId())
        ->getKey('bundle')
      ;
      if (!empty($bundle_key)) {
        $bundle_id = $this->getMappedEntityBundleId();
        $query->condition($bundle_key, $bundle_id);
        $count_query->condition($bundle_key, $bundle_id);
      }

      // Pagination.
      if (isset($parameters['#pageSize']) && (0 < $parameters['#pageSize'])) {
        $page_size = $parameters['#pageSize'];
        $page = empty($parameters['#page']) ? 0 : $parameters['#page'];
      }
      // Get total number of entities matching filters.
      $item_count = intval($count_query->count()->execute());
      // Do not apply pagination now when there are post-filters.
      if (empty($post_filters) && !empty($page_size)) {
        // Note: range (pagination) is not applied on the total count.
        $query->range($page, $page_size);
      }
      elseif (!empty($post_filters)) {
        $item_count = 0;
      }
      $current_page = 0;
      // Get IDs of selected entity range.
      $ids = $query->execute();
      // Load entity instances.
      $entities = $storage->loadMultiple($ids);
    }

    // Initialize result array.
    $result = [];
    // Check which fields are arrays.
    $array_fields = [];
    list($version, $active_def, $datatype_name) = $this->parseId();
    $brapi_definition = brapi_get_definition($version, $active_def);
    $brapi_fields =
      $brapi_definition['data_types'][$datatype_name]['fields']
      ?? []
    ;
    foreach ($brapi_fields as $field_name => $field_def) {
      if (substr($field_def['type'], -2) === '[]') {
        $array_fields[$field_name] = TRUE;
      }
    }

    // Now translate each Drupal entity into a BrAPI datatype.
    foreach ($entities as $id => $entity) {
      // @todo: Warn if the entity type does not match the BrAPI mapping.
      $brapi_data = [];
      foreach ($this->mapping as $brapi_field => $drupal_mapping) {
        if (!empty($drupal_mapping)
            && !empty($drupal_mapping['field'])
        ) {
          // Skip hidden fields such as parent object id in sub-mapping.
          if (!empty($drupal_mapping['hidden']) && (empty($parameters['#include_hidden']))) {
            continue 1;
          }

          try {
            // Check mapping type.
            if ('_submapping' == $drupal_mapping['field']) {
              $brapi_data[$brapi_field] = NULL;
              // Get sub-content to use.
              if (empty($drupal_mapping['subcontent'])) {
                // Just use current content.
                $sub_entities = $entity;
              }
              else {
                $field_name = $drupal_mapping['subcontent'];
                if ('_custom' == $field_name) {
                  // Get field value(s).
                  if (!isset($entity_array)) {
                    $entity_array = $this->entityToArray($entity);
                  }
                  // Issue: the design expects an entity and we provide a field.
                  // @todo Rethink the thing. Remove "custom"? or:
                  // Consider the returned value as an identifier to load an
                  // entity of the type given by the corresponding BrAPI mapping.
                  $sub_entities = $this->parseCustomValue(
                    $drupal_mapping['custom'],
                    $entity_array,
                    $drupal_mapping['is_json'],
                    'Invalid JSONPath for BrAPI field ' . $brapi_field
                  );
                }
                else {
                  // Get referenced entities from field.
                  $sub_entities = $entity->get($field_name)->referencedEntities();
                  // Check if the field should return a single value or is an
                  // array as we manage returned values according to the type.
                  if ('[]' != substr($brapi_fields[$brapi_field]['type'], -2)) {
                    // Return a single value.
                    if (empty($sub_entities)) {
                      $sub_entities = NULL;
                    }
                    else {
                      $sub_entities = current($sub_entities);
                    }
                  }
                }
              }

              if (!empty($sub_entities)) {
                // Here $sub_entities is expected to contain a single
                // ContentEntity or an array of ContentEntity. Depending on the
                // type (single/array), the returned value will also be a single
                // field value (which may be complex) or an array of those.

                // Get sub-mapping type.
                if ('custom' == $drupal_mapping['submapping']) {
                  // Custom sub-mapping, load datatype entity.
                  $sub_datatype_id = $this->id . '-' . $brapi_field;
                  $sub_datatype = $mapping_loader->load($sub_datatype_id);
                }
                else {
                  // Field mapped to an other BrAPI mapping.
                  $sub_datatype = $mapping_loader->load($drupal_mapping['submapping']);
                }
                // @todo: make sure we won't get into an infinite loop if a
                // submapping is also submapping current content.
                // We should check if a given entity with a given mapping has
                // already been processed that way. A "processed" chain should
                // be added to $parameters and checked.
                // Get data from sub-mapping.
                if (!empty($sub_datatype)) {
                  if (is_array($sub_entities)) {
                    $brapi_data[$brapi_field] = [];
                    foreach ($sub_entities as $sub_entity) {
                      $brapi_data[$brapi_field][] = $sub_datatype->getBrapiData(['#entity' => $sub_entity])['entities'];
                    }
                  }
                  else {
                    $brapi_data[$brapi_field] = $sub_datatype->getBrapiData(['#entity' => $sub_entities])['entities'];
                  }
                }
                else {
                  // No corresponding sub-mapping found.
                  // @todo: improve logs.
                  \Drupal::logger('brapi')->warning(
                    'No corresponding sub-mapping found (%brapi_mapping) for BrAPI field %brapi_field of %datatype.',
                    [
                      '%brapi_mapping' => $sub_datatype_id,
                      '%brapi_field' => $brapi_field,
                      '%datatype' => $this->label,
                    ]
                  );
                  $brapi_data[$brapi_field] = NULL;
                }

              }
              else {
                // No sub-entity found, nothing to return.
                $brapi_data[$brapi_field] = NULL;
              }
            }
            elseif ('_custom' == $drupal_mapping['field']) {
              // Custom value.
              if (!isset($entity_array)) {
                $entity_array = $this->entityToArray($entity);
              }
              $brapi_data[$brapi_field] = $this->parseCustomValue(
                $drupal_mapping['custom'],
                $entity_array,
                $drupal_mapping['is_json'],
                'Invalid JSONPath ("' . $drupal_mapping['custom'] . '") for field "' . $brapi_field . '" of ' . $this->label
              );
            }
            else {
              // BrAPI field mapped to an entity field, get its value.
              $field = $entity->get($drupal_mapping['field']);
              // Check if field is an entity reference.
              if ($field->getFieldDefinition()->getType() == 'entity_reference') {
                // We got referenced entities.
                // Add all referenced entities as arrays.
                $brapi_data[$brapi_field] = array_map(
                  function ($ref_entity) { return $ref_entity->toArray(); },
                  $field->referencedEntities()
                );
              }
              else {
                // Regular field, get it as string.
                if (!empty($array_fields[$brapi_field])) {
                  $brapi_data[$brapi_field] = $field->getValue();
                }
                else {
                  $brapi_data[$brapi_field] = $field->getString();
                }
              }
            }
          }
          catch (\InvalidArgumentException $e) {
            // Warn for invalid mapping.
            \Drupal::logger('brapi')->warning(
              'Invalid datatype field mapping for BrAPI object field "%brapi_field" to Drupal entity field "%drupal_field".',
              [
                '%brapi_field'  => $brapi_field,
                '%drupal_field' => $drupal_mapping['field'],
              ]
            );
          }
          catch (\Throwable $e) {
            \Drupal::logger('brapi')->error($e);
          }

          // Check expected structure: array or not?
          if (!empty($array_fields[$brapi_field])
              && (!is_array($brapi_data[$brapi_field]))
              && isset($brapi_data[$brapi_field])
          ) {
            // Should return an array.
            $brapi_data[$brapi_field] = [$brapi_data[$brapi_field]];
          }
          elseif (empty($array_fields[$brapi_field])
              && (is_array($brapi_data[$brapi_field]))
          ) {
            // Should return a single value.
            if (empty($brapi_data[$brapi_field])) {
              // Got nothing anyway.
              $brapi_data[$brapi_field] = NULL;
            }
            else {
              // Should not return an array, make sure it is a sequencial array.
              if (array_keys($brapi_data[$brapi_field]) === range(0, count($brapi_data[$brapi_field]) - 1)) {
                // Get first array value.
                $brapi_data[$brapi_field] = reset($brapi_data[$brapi_field]);
              }
            }
          }
        }
        else {
          // No mapping for that field.
          $brapi_data[$brapi_field] = NULL;
        }
      }

      // Process post-filtering if needed.
      if (!empty($post_filters)) {
        // Apply post-filters and pagination.
        foreach ($post_filters as $field => $value) {
          // Make sure entity has this field.
          if (!array_key_exists($field, $brapi_data)) {
            // Entity does not have a value for this field, skip entity.
            continue 2;
          }
          if (is_array($value)) {
            // Filter on a list of possible values.
            if (empty(($value))) {
              // Empty filter list, skip filter.
              continue 1;
            }
            // Filter value is a non-empty array.
            if (is_array($brapi_data[$field])) {
              // Entity field contains an array of values as well.
              foreach ($brapi_data[$field] as $entity_value) {
                if (in_array($entity_value, $value)) {
                  // Matched a value, skip filter.
                  continue 2;
                }
              }
              // No value matched, skip entity.
              continue 2;
            }
            elseif (!in_array($brapi_data[$field], $value)) {
              // Filter value is an array and entity value is a single value not
              // in that array. Unmatched value, skip that entity.
              continue 2;
            }
          }
          elseif (isset($value) && ('' != $value)) {
            // Filter value is a single non-empty (NULL and empty string) value.
            if (!is_array($brapi_data[$field])) {
              if ($value != $brapi_data[$field]) {
                // Filter value and entity value are single values but are
                // different. Unmatched value, skip that entity.
                continue 2;
              }
            }
            elseif (!in_array($value, $brapi_data[$field])) {
              // Entity field contains an array of values.
              // Filter value does not match any of the entity values (array).
              // Unmatched value, skip that entity.
              continue 2;
            }
          }
        }
        // Entity passed filtration.
        $result[] = $brapi_data;
        // Manage pagination.
        if (!empty($page_size)) {
          ++$item_count;
          if (count($result) == $page_size) {
            // We got a full page.
            if ($current_page == $page) {
              // We got our page.
              // $post_result is only filled when a page is complete and
              // correspond to the requested page.
              $post_result = $result;
            }
            // Empty list and proceed to next page filling.
            $result = [];
            ++$current_page;
          }
        }
      }
      else {
        // No post-filtering.
        $result[] = $brapi_data;
      }
    }

    // Manage post-filtering pagination.
    if (!empty($post_filters) && !empty($page_size)) {
      // Make sure the client did not ask a higher page.
      if ($current_page < $page) {
        // Requested page does not exist, return an empty set.
        $post_result = [];
      }
      elseif (empty($post_result) && ($current_page == $page)) {
        // The last page may not be complete and $post_result may not be filled.
        // Get what's left.
        $post_result = $result;
      }
      // Get post-filtering results.
      $result = $post_result;
    }

    return ['total_count' => $item_count, 'entities' => $result];
  }

  /**
   * Convert an entity into an array.
   *
   * This method also resolves first level referenced entities to their content
   * as an array as well. Therefore, an entity reference field which should be
   * normally turned into a structure like:
   *
   *   ["ref_field_name" => [["target_id" => 1234]]]
   *
   * would now be turned into:
   *
   *   ["ref_field_name" => [["target_id" => 1234, "value" => <the referenced entity as an array>]]]
   *
   * @param EntityInterface $entity
   *   The entity to convert.
   *
   * @return array
   *   The entity content as an array with referenced entities as array as well.
   */
  protected function entityToArray(
    EntityInterface $entity
  ) :array {
    $entity_array = $entity->toArray();
    // Turn referenced entities into their array.
    foreach (array_keys($entity_array) as $field_name) {
      $field = $entity->get($field_name);
      if ($field->getFieldDefinition()->getType() == 'entity_reference') {
        foreach ($field->referencedEntities() as $index => $ref_entity) {
          $entity_array[$field_name][$index]['value'] = $ref_entity->toArray();
        }
      }
    }

    return $entity_array;
  }

  /**
   * Parse custom field value that may contain JSON path items and be JSON data.
   *
   * @param string $custom_value
   *   The custom string containing static text, JSON Path expression or any
   *   combination of both.
   * @param array $entity_array
   *   An array representing the entity content from wich data should be
   *   extracted.
   * @param $is_json
   *   If not empty, the result string is parsed as JSON and the parsed
   *   data structure is returned instead of a sting.
   * @param string $invalid_jp_message
   *   Message to display in case of invalid JSON Path.
   *
   * @return string or array
   *   The JSON Path parsed string is returned or its corresponding structure as
   *   an array if $is_json is not empty.
   *   NULL is returned in case of invalid JSON data (and non-empty $is_json).
   */
  protected function parseCustomValue(
    string $custom_value,
    array $entity_array,
    $is_json,
    string $invalid_jp_message = 'Invalid JSONPath'
  ) {
    // Check for JSON path to replace.
    if (preg_match_all('/\$(?:\.?\*|\.\.|\.\w+|\[\'\w+\'(?:\s*,\s*\'\w+\')*\]|\[-?\d+(?:\s*,\s*-?\d+)*\]|\[-?\d*:-?\d*\]|\[\*\])+/', $custom_value, $matches)) {
      foreach ($matches[0] as $match) {
        try {
          $json_object = new JsonObject($entity_array, TRUE);
          $jpath_value = $json_object->get($match);
          if (!empty($is_json)) {
            $custom_value = str_replace($match, json_encode($jpath_value), $custom_value);
          }
          else {
            if (!is_string($jpath_value)) {
              $jpath_value = json_encode($jpath_value);
            }
            $custom_value = str_replace($match, $jpath_value, $custom_value);
          }
        }
        catch (InvalidJsonException | InvalidJsonPathException $e) {
          // JSONPath mapping failed. Report.
          \Drupal::logger('brapi')->warning(
            $invalid_jp_message
            . ': '
            . $e
            . "\nData: "
            . print_r($entity_array, TRUE)
          );
        }
      }
    }
    // Manage JSON data in a custom string.
    if (!empty($is_json)) {
      // Treate as JSON.
      $json_value = json_decode($custom_value, TRUE);
      if (!isset($json_value) && !empty($custom_value)) {
        \Drupal::logger('brapi')->warning(
          'Failed to parse JSON value "'
          . $custom_value
          . '": '
          . json_last_error_msg()
        );
      }
      $custom_value = $json_value;
    }
    return $custom_value;
  }

  /**
   * Returns the related Drupal content field name.
   *
   * Returns the associated Drupal content field name of a given BrAPI field
   * name.
   *
   * @param string $brapi_field_name
   *   A BrAPI field name.
   * @param bool $auto_adjust_field_name
   *   If TRUE, also test singular and plural versions of the field name.
   *
   * @return bool|string
   *   The associated Drupal field name if mapped, an empty string if mapped to
   *   a complex value (like a custom value) or FALSE if no mapping was set.
   */
  public function getDrupalMappedField(string $brapi_field_name, bool $auto_adjust_field_name = FALSE) {
    $mapped_field = $this->mapping[$brapi_field_name]['field'] ?? FALSE;

    if ($auto_adjust_field_name) {
        if (FALSE === $mapped_field) {
          $mapped_field = $this->mapping[brapi_get_term_singular($brapi_field_name)]['field'] ?? FALSE;
        }
        if (FALSE === $mapped_field) {
          $mapped_field = $this->mapping[brapi_get_term_plural($brapi_field_name)]['field'] ?? FALSE;
        }
    }

    // Take into account special mappings.
    if (!empty($mapped_field) && ('_custom' == $mapped_field)) {
      $mapped_field = '';
    }
    if (!empty($mapped_field) &&("_submapping" == $mapped_field)) {
      $mapped_field = $this->mapping[$brapi_field_name]['subcontent'];
    }

    return $mapped_field;
  }

  /**
   * Create or update a BrAPI record.
   *
   * @param array $parameters
   *   An array of keyed values used to create or update an object.
   *   Special keys starting with '#' are not treated as field values.
   *   - '#is_new': if TRUE, a new object should be created but if there is any
   *     existing object with the same identifier, it will raise an exception.
   * @return array
   * Returns the new BrAPI data if recorded.
   * @throw \Drupal\brapi\Exception\BrapiObjectAlreadyExistsException
   * @throw \Drupal\brapi\Exception\BrapiObjectException
   * @throw \Drupal\brapi\Exception\BrapiStorageException
   */
  public function saveBrapiData(array $parameters) :array {
    $storage = \Drupal::service('entity_type.manager')
      ->getStorage($this->getMappedEntityTypeId())
    ;
    if (empty($storage)) {
      \Drupal::logger('brapi')->error(
        "No storage available for content type '" . $this->contentType . "'."
      );
      throw new BrapiStorageException(
        'Failed to load the storage engine for data type '
        . $this->getBrapiDatatype()
        . '.'
      );
    }
    
    $module_handler = \Drupal::moduleHandler();
    $data_type_hook = 'brapi_' . strtolower($this->getBrapiDatatype());
    // Allow other module to alter saved data before saving.
    if ($module_handler->hasImplementations($data_type_hook . '_save_alter')) {
      $module_handler->alter($data_type_hook . '_save', $parameters, $this, $storage);
    }

    $id_field_name = $this->getBrapiIdField();
    if (!empty($parameters['#is_new'])) {
      // New record.
      if (!empty($id_field_name)
        && !empty($parameters[$id_field_name])
      ) {
        // Trying to create a new object with a specific identifier.
        $id_filter = [$id_field_name => $parameters[$id_field_name]];
        // Make sure it does not exist already.
        if (!empty($this->getBrapiData($id_filter)['entities'])) {
          throw new BrapiObjectAlreadyExistsException(
            "Cannot create a new "
            . $this->getBrapiDatatype()
            . " object. Another object with the same identifier already exists ("
            . $id_field_name
            . '='
            . $parameters[$id_field_name]
            . ")."
          );
        }
      }
      // Create Drupal entity.
      $drupal_data = [];
      // If mapped to a bundle, specify it.
      $bundle_key = \Drupal::entityTypeManager()
        ->getDefinition($this->getMappedEntityTypeId())
        ->getKey('bundle')
      ;
      if (!empty($bundle_key)) {
        $drupal_data[$bundle_key] = $this->getMappedEntityBundleId();
      }
      foreach ($parameters as $parameter => $value) {
        // Get Drupal field mapping.
        $field_name = $this->getDrupalMappedField($parameter, TRUE);
        if ((FALSE !== $field_name)
          && ('' !== $field_name)
        ) {
          $drupal_data[$field_name] = $value;
        }
        elseif ('#' != $parameter[0]) {
          \Drupal::logger('brapi')->warning(
            'BrAPI field "' . $parameter . '" can not be mapped to content type "' . $this->contentType . '". Field value ignored.'
          );
        }
      }
      $entity = $storage->create($drupal_data);
    }
    else {
      // Update.
      if (empty($id_field_name)) {
        throw new BrapiStorageException(
          'Unable to update data type '
          . $this->getBrapiDatatype()
          . ': no identifier field available.'
        );
      }
      if (empty($parameters[$id_field_name])) {
        throw new BrapiObjectException(
          "Cannot update "
          . $this->getBrapiDatatype()
          . " object. No object identifier provided ("
          . $id_field_name
          . ")."
        );
      }
      // Load Drupal entity.
      $query = $storage->getQuery();
      $query->accessCheck(TRUE);
      $drupal_id_field_name = $this->getDrupalMappedField($id_field_name, TRUE);
      $query->condition($drupal_id_field_name, $parameters[$id_field_name]);
      $drupal_id = current($query->execute());
      if (!empty($drupal_id)) {
        $entity = $storage->load($drupal_id);
      }
      if (empty($entity)) {
        throw new BrapiObjectException(
          "Cannot update "
          . $this->getBrapiDatatype()
          . " object. Failed to load existing object ("
          . $id_field_name
          . '='
          . $parameters[$id_field_name]
          . ")."
        );
      }

      // Update field values.
      foreach ($parameters as $parameter => $value) {
        // Get Drupal field mapping.
        $field_name = $this->getDrupalMappedField($parameter, TRUE);
        if ((FALSE !== $field_name)
          && ('' !== $field_name)
        ) {
          $entity->set($field_name, $value);
        }
        else {
          \Drupal::logger('brapi')->warning(
            'BrAPI field "' . $parameter . '" can not be mapped to content type "' . $this->contentType . '". Field value ignored.'
          );
        }
      }
    }
    // Save changes.
    $storage->save($entity);

    $saved_data = [];
    $updated_entities = $this->getBrapiData($parameters)['entities'];
    if (1 == count($updated_entities)) {
      $saved_data = $updated_entities[0];
    }

    return $saved_data;
  }

  /**
   * Delete BrAPI record(s).
   *
   * @param array $parameters
   *   An array of parameters used to identify the object(s) to delete.
   *
   * @return array
   * An array of deleted object identifiers if deleted or [] if failed.
   *
   * @throw \Drupal\brapi\Exception\BrapiStorageException
   * @see https://api.drupal.org/api/drupal/core%21includes%21common.inc/10
   */
  public function deleteBrapiData(array $parameters) :array {

    $storage = \Drupal::service('entity_type.manager')
      ->getStorage($this->getMappedEntityTypeId())
    ;
    if (empty($storage)) {
      \Drupal::logger('brapi')->error(
        "No storage available for content type '" . $this->contentType . "'."
      );
      throw new BrapiStorageException(
        'Failed to load the storage engine for data type '
        . $this->getBrapiDatatype()
        . '.'
      );
    }

    // Load Drupal entities.
    $query = $storage->getQuery();
    $query->accessCheck(TRUE);
    foreach ($parameters as $parameter => $value) {
      $drupal_field_name = $this->getDrupalMappedField($parameter, TRUE);
      if (!empty($drupal_field_name)) {
        $query->condition($drupal_field_name, $value);
      }
    }
    $drupal_ids = $query->execute();
    // Delete.
    $removed = [];
    foreach ($drupal_ids as $drupal_id) {
      $entity = $storage->load($drupal_id);
      if (empty($entity)) {
        throw new BrapiObjectException(
          "Cannot update "
          . $this->getBrapiDatatype()
          . " object. Failed to load existing object ("
          . $this->getMappedEntityTypeId()
          . ' id='
          . $drupal_id
          . ")."
        );
      }
      $removed[] = $entity->id();
      $storage->delete([$entity]);
    }

    return $removed;
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
  protected $brapiSubFields;

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
