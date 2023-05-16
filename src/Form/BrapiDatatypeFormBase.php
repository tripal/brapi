<?php

namespace Drupal\brapi\Form;

use Drupal\Core\Entity\ContentEntityType;
use Drupal\Core\Entity\EntityFieldManagerInterface;
use Drupal\Core\Entity\EntityForm;
use Drupal\Core\Entity\EntityStorageInterface;
use Drupal\Core\Entity\EntityTypeBundleInfoInterface;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Link;
use Drupal\Core\Url;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Class BrapiDatatypeFormBase.
 */
class BrapiDatatypeFormBase extends EntityForm {

  /**
   * An entity query factory for the BrAPI Datatype Mapping entity type.
   *
   * @var \Drupal\Core\Entity\EntityStorageInterface
   */
  protected $entityStorage;

  /**
   * The entity type manager.
   *
   * @var \Drupal\Core\Entity\EntityTypeManagerInterface
   */
  protected $entityTypeManager;

  /**
   * The entity type bundle info.
   *
   * @var \Drupal\Core\Entity\EntityTypeBundleInfoInterface
   */
  protected $entityTypeBundleInfo;

  /**
   * The entity field manager.
   *
   * @var \Drupal\Core\Entity\EntityFieldManagerInterface
   */
  protected $entityFieldManager;

  /**
   * Constructs an BrapiDatatypeFormBase object.
   *
   * @param \Drupal\Core\Entity\EntityStorageInterface $entity_storage
   *   The entity storage.
   * @param \Drupal\Core\Entity\EntityTypeManagerInterface $entity_type_manager
   *   The entity type manager.
   * @param \Drupal\Core\Entity\EntityTypeBundleInfoInterface $entity_type_bundle_info
   *   The entity type bundle info.
   * @param \Drupal\Core\Entity\EntityFieldManagerInterface $entity_field_manager
   *   The entity field manager.
   */
  public function __construct(
    EntityStorageInterface $entity_storage,
    EntityTypeManagerInterface $entity_type_manager,
    EntityTypeBundleInfoInterface $entity_type_bundle_info,
    EntityFieldManagerInterface $entity_field_manager
  ) {
    $this->entityStorage = $entity_storage;
    $this->entityTypeManager = $entity_type_manager;
    $this->entityTypeBundleInfo = $entity_type_bundle_info;
    $this->entityFieldManager = $entity_field_manager;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    $form = new static(
      $container->get('entity_type.manager')->getStorage('brapidatatype'),
      $container->get('entity_type.manager'),
      $container->get('entity_type.bundle.info'),
      $container->get('entity_field.manager')
    );
    $form->setMessenger($container->get('messenger'));
    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state, $mapping_id = '') {
    $form = parent::buildForm($form, $form_state);
    // Adds JS library.
    $form['#attached']['library'][] = 'brapi/brapi.core';

    $brapi_datatype = $this->entity;

    // Build the form.
    if (!empty($mapping_id)) {
      // Adding a new mapping.
      $mapping_name = $mapping_id;
      if (preg_match(BRAPI_DATATYPE_ID_REGEXP, $mapping_id, $matches)) {
        list(, $version, $active_def, $datatype_name, $subfields) = $matches;
        $subfields = array_filter(explode('-', $subfields));
        $mapping_name = $datatype_name . ' for BrAPI v' . $active_def;
      }
      else {
        return [
          'error' => [
            '#type' => 'markup',
            '#markup' => $this->t('ERROR: Invalid data type identifier!'),
          ]
        ];
      }
      if (empty($brapi_datatype->id())) {
        $brapi_datatype->id = $mapping_id;
      }
      if (!empty($subfields)) {
        $form['title'] = [
          '#type' => 'markup',
          '#markup' => $this->t('Data sub-mapping for BrAPI data type %datatype_name subfield(s) %subfields', ['%datatype_name' => $datatype_name, '%subfields' => implode(', ', $subfields)]),
        ];
        $mapping_name = $datatype_name . '.' . implode('.', $subfields) . ' for BrAPI v' . $active_def;
      }
      else {
        $form['title'] = [
          '#type' => 'markup',
          '#markup' => $this->t('Data mapping for BrAPI data type %datatype_name', ['%datatype_name' => $datatype_name]),
        ];
      }
      $form['label'] = [
        '#type' => 'hidden',
        '#value' => $mapping_name,
      ];
      $form['id'] = [
        '#type' => 'hidden',
        '#value' => $mapping_id,
      ];
    }
    else {
      // Editing an existing mapping.
      $mapping_id = $brapi_datatype->id();
      list($version, $active_def, $datatype_name, $subfields) = $brapi_datatype->parseId();
      if (!empty($subfields)) {
        $form['title'] = [
          '#type' => 'markup',
          '#markup' => $this->t('Data sub-mapping for BrAPI data type %datatype_name subfield(s) %subfields', ['%datatype_name' => $datatype_name, '%subfields' => implode(', ', $subfields)]),
        ];
      }
      else {
        $form['title'] = [
          '#type' => 'markup',
          '#markup' => $this->t('Data mapping for BrAPI data type %datatype_name', ['%datatype_name' => $datatype_name]),
        ];
      }

      $form['label'] = [
        '#type' => 'textfield',
        '#title' => $this->t('Label'),
        '#maxlength' => 255,
        '#default_value' => $brapi_datatype->label(),
        '#required' => TRUE,
      ];
      if ($brapi_datatype->isNew()) {
        $form['id'] = [
          '#type' => 'machine_name',
          '#title' => $this->t('Machine name'),
          '#default_value' => $brapi_datatype->id(),
          '#machine_name' => [
            'exists' => [$this, 'exists'],
            'replace_pattern' => '([^a-z0-9_\-\.]+)',
            'error' => 'The machine-readable name must be unique, and can only contain lowercase letters, numbers, underscores, dashes and dots.',
          ],
          '#description' => $this->t('The machine-readable name must be unique, and can only contain lowercase letters, numbers, underscores, dashes and dots.'),
        ];
      }
      else {
        // Used by Javascript.
        $form['id'] = [
          '#type' => 'hidden',
          '#value' => $brapi_datatype->id(),
        ];
      }
    }

    // Check for sub-mapping.
    if (!empty($subfields)) {
      // Get parent mapping datatype.
      $mapping_loader = \Drupal::service('entity_type.manager')
        ->getStorage('brapidatatype')
      ;
      $parent_datatype_id = substr($mapping_id, 0, strrpos($mapping_id, '-'));
      $parent_datatype = $mapping_loader->load($parent_datatype_id);
      if (empty($parent_datatype)) {
        $form['error'] = [
          '#type' => 'markup',
          '#markup' => $this->t('ERROR: Parent mapping (@parent) not found!', ['@parent' => $parent_datatype]),
        ];
        return $form;
      }
      $form['contentType'] = [
        '#type' => 'hidden',
        '#value' => $parent_datatype->contentType,
      ];
      $form_state->setValue('contentType', $parent_datatype->contentType);
    }
    else {
      // Display the list of selectable content types.
      $content_options = $this->getEntityTypeIdOptions($form, $form_state);
      $form['contentType'] = [
        '#type' => 'select',
        '#options' => $content_options,
        '#title' => $this->t('Select the associated Drupal content type'),
        '#default_value' => $form_state->getValue('contentType') ?? $brapi_datatype->contentType ?? '',
        '#required' => TRUE,
        '#ajax' => [
          'callback' => [get_class($this), 'buildAjaxFieldMappingForm'],
          'wrapper' => 'brapi-field-mapping-form',
          'method' => 'replace',
          'effect' => 'fade',
        ],
      ];
    }

    $form['mapping'] = [
      '#type' => 'fieldset',
      '#title' => $this->t('Mapping'),
      '#attributes' => ['id' => 'brapi-field-mapping-form'],
      '#tree' => TRUE,
    ];
    $form['mapping'] += $this->getFieldMappingForm($form, $form_state);

    // If we are mapping subfields, keep source object id as a possible
    // filter.
    if (!empty($subfields)) {
      $brapi_definition = brapi_get_definition($version, $active_def);
      $id_field = brapi_get_datatype_identifier(
        $datatype_name,
        $brapi_definition
      );
      $id_field_def =
        $brapi_definition['data_types'][$datatype_name]['fields'][$id_field]
        ?? NULL
      ;
      if (!empty($id_field_def) && !empty($parent_datatype->mapping[$id_field])) {
        $form['mapping'][$id_field]['hidden'] = [
          '#type' => 'hidden',
          '#value' => TRUE,
        ];
        foreach ($parent_datatype->mapping[$id_field] as $param => $value) {
          $form['mapping'][$id_field][$param] = [
            '#type' => 'hidden',
            '#value' => $value,
          ];
        }
      }
    }

    $form['actions']['brapi-mapping-export'] = [
      '#type' => 'button',
      '#value' => $this->t('Export'),
      // Place the button after save/update/delete buttons.
      '#weight' => 11,
      '#attributes' => [
        'onclick' => 'return false;'
      ],
      '#attached' => array(
        'library' => array(
          'brapi/brapi.core',
        ),
      ),
    ];

    $form['actions']['brapi-mapping-import'] = [
      '#type' => 'button',
      '#value' => $this->t('Import'),
      // Place the button after save/update/delete/export buttons.
      '#weight' => 12,
      '#attributes' => [
        'onclick' => 'return false;',
      ],
      '#attached' => [
        'library' => [
          'brapi/brapi.core',
        ],
      ],
    ];

    return $form;
  }

  /**
   * Handles switching the selected content type.
   *
   * @param array $form
   *   The current form array.
   * @param \Drupal\Core\Form\FormStateInterface $form_state
   *   The current form state.
   *
   * @return array
   *   The part of the form to return as AJAX.
   */
  public static function buildAjaxFieldMappingForm(array $form, FormStateInterface $form_state) {
    // The work is already done in form(), where we rebuild the entity according
    // to the current form values and then create the storage client
    // configuration form based on that. So we just need to return the relevant
    // part of the form
    // here.
    return $form['mapping'];
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    parent::submitForm($form, $form_state);
    // Clear cache.
    \Drupal::cache('brapi_search')->invalidateAll();
    $this->messenger()->addMessage($this->t('BrAPI search cache has been cleared.'));
  }

  /**
   * {@inheritdoc}
   */
  public function exists($entity_id, array $element, FormStateInterface $form_state) {
    $query = $this->entityStorage->getQuery();
    $result = $query
      ->condition('id', $element['#field_prefix'] . $entity_id)
      ->execute()
    ;
    return (bool) count($result);
  }

  /**
   * {@inheritdoc}
   */
  protected function actions(array $form, FormStateInterface $form_state) {
    $actions = parent::actions($form, $form_state);
    $actions['submit']['#value'] = $this->t('Save');
    return $actions;
  }

  /**
   * {@inheritdoc}
   */
  public function save(array $form, FormStateInterface $form_state) {
    $brapi_datatype = $this->getEntity();
    $is_new = $brapi_datatype->isNew();
    $status = $brapi_datatype->save();
    $url = $brapi_datatype->toUrl();
    $edit_link = Link::fromTextAndUrl($this->t('Edit'), $url)->toString();

    if ($status == SAVED_UPDATED) {
      // If we edited an existing entity...
      $this->messenger()->addMessage($this->t('BrAPI Datatype Mapping %label has been updated.', ['%label' => $brapi_datatype->label()]));
      $this->logger('brapi')->notice('BrAPI Datatype Mapping %label has been updated.', ['%label' => $brapi_datatype->label(), 'link' => $edit_link]);
    }
    else {
      // If we created a new entity...
      $this->messenger()->addMessage($this->t('BrAPI Datatype Mapping %label has been added.', ['%label' => $brapi_datatype->label()]));
      $this->logger('brapi')->notice('BrAPI Datatype Mapping %label has been added.', ['%label' => $brapi_datatype->label(), 'link' => $edit_link]);
    }

    // Redirect the user back to the data type management page.
    if ($is_new) {
      $form_state->setRedirect('entity.brapidatatype.edit_form', ['brapidatatype' => $brapi_datatype->id(),]);
    }
  }

  /**
   * Gets the annotation entity type id options.
   *
   * @param array $form
   *   The current form array.
   * @param \Drupal\Core\Form\FormStateInterface $form_state
   *   The current form state.
   *
   * @return array
   *   Associative array of entity type labels, keyed by their ids.
   */
  public function getEntityTypeIdOptions(array $form, FormStateInterface $form_state) {
    $options = [];

    $definitions = $this->entityTypeManager->getDefinitions();
    /* @var \Drupal\Core\Entity\EntityTypeInterface $definition */
    foreach ($definitions as $entity_type_id => $definition) {
      if ($definition instanceof ContentEntityType) {
        // Check bundles.
        $bundles = $this->entityTypeBundleInfo->getBundleInfo($entity_type_id);
        if (empty($bundles)
            || ((1 == count($bundles))
                && (array_keys($bundles)[0] == $entity_type_id))
        ) {
          $options[$entity_type_id . ':' . $entity_type_id] = $definition->getLabel();
        }
        else {
          // We have multiple bundles.
          $bundle_options = [];
          foreach ($bundles as $bundle_id => $bundle_def) {
            $bundle_options[$entity_type_id . ':' . $bundle_id] = $bundle_def['label'];
          }
          $options[''.$definition->getLabel()] = $bundle_options;
        }
      }
    }
    return $options;
  }

  /**
   * Gets the data type field mapping form.
   *
   * Returns the list of BrAPI data type fields with their Drupal content field
   * mapping in a form.
   *
   * Note: I've thought about using External Entities Field Mapper but it is
   * too External Entities based; For instance, see
   * FieldMapperBase::getMappableFields():
   * ```
   *   $derived_entity_type = $this->getExternalEntityType()->getDerivedEntityType();
   * ```
   *
   * @param array $form
   *   The current form array.
   * @param \Drupal\Core\Form\FormStateInterface $form_state
   *   The current form state.
   *
   * @return array
   *   A form array.
   */
  public function getFieldMappingForm(array $form, FormStateInterface $form_state) {
    $base_types = ['string', 'integer', 'boolean', 'number', ];
    $mapping_form = [];
    $brapi_datatype = $this->entity;

    // Get data type mapping entities.
    $mapping_loader = \Drupal::service('entity_type.manager')
      ->getStorage('brapidatatype')
    ;

    // Get mapped content field list.
    $content_type =
      $form_state->getValue('contentType')
      ?? $brapi_datatype->contentType
    ;
    if (empty($content_type)) {
      return [
        'error' => [
          '#type' => 'markup',
          '#markup' => $this->t('ERROR: Missing associated content type for mapping @mapping!', ['@mapping' => $brapi_datatype->id()]),
        ]
      ];
    }

    // Generate the lists of field mapping options.
    $string_field_options = [];
    $entityref_field_options = [];
    $custom_field_option = ['_custom' => '[Custom value]',];
    if (preg_match('/^(.+):(.+)$/', $content_type, $matches)) {
      list(, $entity_type_id, $bundle_id) = $matches;
      if ($entity_type_id && $bundle_id) {
        $fields = $this->entityFieldManager->getFieldDefinitions(
          $entity_type_id,
          $bundle_id
        );
        foreach ($fields as $field_id => $field) {
          if ($field->getType() == 'entity_reference') {
            $string_field_options['References'][$field_id] =
              $field->getLabel()
              . ' ('
              . $field->getName()
              . ' referencing '
              . $field->getSetting('target_type')
              . ')'
            ;
            $entityref_field_options['object'][$field_id] =
              $field->getLabel()
              . ' ('
              . $field->getName()
              . ' referencing '
              . $field->getSetting('target_type')
              . ')'
            ;
            $entityref_field_options[$field->getSetting('target_type')][$field_id] =
              $field->getLabel()
              . ' ('
              . $field->getName()
              . ' referencing '
              . $field->getSetting('target_type')
              . ')'
            ;
          }
          else {
            $string_field_options['Fields'][$field_id] =
              $field->getLabel() . ' (' . $field->getName() . ')'
            ;
          }
        }
      }
    }
    // ksort($string_field_options['Fields']);
    // ksort($string_field_options['References']);

    // Build BrAPI data type field list.
    list($version, $active_def, $datatype_name, $subfields) =
      $brapi_datatype->parseId();
    if (!empty($datatype_name)) {
      $brapi_definition = brapi_get_definition($version, $active_def);
      // If we are mapping a subfield structure, iterate on subfields to only
      // get the subfield structure to map (instead of the whole BrAPI data
      // type).
      while (!empty($subfields)) {
        $subfield = array_shift($subfields);
        if (empty($brapi_definition['data_types'][$datatype_name]['fields'][$subfield])) {
          $this->logger('brapi')->error('Invalid sub-mapping identifier: %id. Stopping at sub-field "%field" of data type "%datatype_name".', ['%id' => $brapi_datatype->id(), '%field' => $subfield, '%datatype_name' => $datatype_name, ]);
          return [
            $field_name => [
              '#type' => 'markup',
              '#markup' => $this->t(
                'ERROR: Invalid sub-mapping identifier "@id"!',
                ['@id' => $brapi_datatype->id()]
              ),
            ],
          ];
        }
        $datatype_name = $brapi_definition['data_types'][$datatype_name]['fields'][$subfield]['type'];
        $datatype_name = rtrim($datatype_name, '[]');
      }
      $brapi_fields =
        $brapi_definition['data_types'][$datatype_name]['fields']
        ?? []
      ;
      // Loop on BrAPI fields to map.
      foreach ($brapi_fields as $field_name => $field_def) {
        $field_type = $field_def['type'];
        $required = $field_def['required'];
        $base_datatype = $type = rtrim($field_type, '[]');
        $mapping_form[$field_name] = [
          '#type' => 'details',
          '#title' => $this->t(
            'BrAPI field %field_name mapping',
            ['%field_name' => $field_name,]
          ),
          '#open' => !empty($brapi_datatype->mapping[$field_name]['field']),
          '#tree' => TRUE,
        ];

        // Generate select box for field mapping.
        $mapping_form[$field_name]['field'] = [
          '#type' => 'select',
          '#title' => $this->t(
            'Map BrAPI field %field_name (%field_type) to',
            ['%field_name' => $field_name, '%field_type' => $field_type]
          ),
          '#options' => [],
          '#default_value' => $brapi_datatype->mapping[$field_name]['field'] ?? '',
          '#required' => $required,
          '#empty_value' => '',
          '#attributes' => [
            'id' => 'edit-mapping-' . $field_name . '-field',
          ],
        ];

        // Check for simple values (string, integers, etc.)
        if (in_array($base_datatype, $base_types)) {
          $mapping_form[$field_name]['field']['#options'] =
            $string_field_options
            + $custom_field_option
          ;
        }
        elseif ('object' == $base_datatype) {
          $options =
            $string_field_options
            + $custom_field_option
          ;
          // @todo: add object fields mapping?
          // note: some object structure may depend on another field value;
          //   for instance "GeoJSON Geometry" object content depends on it
          //   "GeoJSON Type" wich can be a point or a polygone.
          //   It might not be possible to provide a structure.
          $mapping_form[$field_name]['field']['#options'] = $options;
        }
        else {
          // Complex types and BrAPI datatypes.
          // Generate a select box for entity reference mapping and sub-mapping.
          $options =
            [
              '_submapping' => '[Sub-mapping]',
            ]
            + $string_field_options
            + $custom_field_option
          ;

          // Offer sub-mapping.
          $submapping_type_options = [
            'custom' => 'Custom sub-mapping',
          ];
          // @todo: We could add "custom field" if used to return the ID of an
          // entity (of type given by the selected sub-mapping) to load.
          // $submapping_content_options = $entityref_field_options['object'] + $custom_field_option;
          $submapping_content_options = $entityref_field_options['object'];
          // Select sub-mapping type: existing BrAPI datatype mapping or custom.
          // Check if a BrAPI datatype-specific mapping is available.
          $base_datatype = brapi_is_reference_to_datatype($field_name, $datatype_name, $brapi_definition);
          if ($base_datatype) {
            $target_datatype_id = brapi_generate_datatype_id($base_datatype, $version, $active_def);
            $other_mapping = $mapping_loader->load($target_datatype_id);
          }
          if (!empty($other_mapping)) {
            // The datatype is mapped to a content type.
            $submapping_type_options[$other_mapping->id()] = $this->t(
              'Use %brapi_mapping mapping',
              ['%brapi_mapping' => $other_mapping->getMappedEntityTypeId(),]
            );
            // We could force the use of the corresponding identifier...
            // or set it as default?
            // // Check if we got an entity field that can reference that content type.
            // if (!empty($entityref_field_options[$other_mapping->getMappedEntityTypeId()])) {
            //   $submapping_content_options =
            //     $entityref_field_options[$other_mapping->getMappedEntityTypeId()]
            //     + $custom_field_option
            //   ;
            // }
          }
          $mapping_form[$field_name]['submapping'] = [
            '#type' => 'select',
            '#title' => $this->t('Select sub-mapping type'),
            '#options' => $submapping_type_options,
            '#default_value' => $brapi_datatype->mapping[$field_name]['submapping'] ?? 'custom',
            '#attributes' => [
              'id' => 'edit-mapping-' . $field_name . '-submapping',
            ],
            '#states' => [
              'visible' => [
                ':input[id="edit-mapping-' . $field_name . '-field"]' => ['value' => '_submapping'],
              ],
            ],
          ];

          // Check if a custom sub-mapping is available.
          $submapping_datatype_id = $brapi_datatype->id() . '-' . $field_name;
          $submapping = $mapping_loader->load($submapping_datatype_id);
          // Can't use '#states' in  'link' element due to issue:
          // https://www.drupal.org/project/drupal/issues/2820586
          $mapping_form[$field_name]['custom_submapping'] = [
            '#type' => 'fieldset',
            '#states' => [
              'visible' => [
                ':input[id="edit-mapping-' . $field_name . '-field"]' => ['value' => '_submapping'],
                'and',
                ':input[id="edit-mapping-' . $field_name . '-submapping"]' => ['value' => 'custom'],
              ],
            ],
          ];
          if (!empty($other_mapping)) {
            $mapping_form[$field_name]['other_submapping'] = [
              '#type' => 'fieldset',
              '#states' => [
                'visible' => [
                  ':input[id="edit-mapping-' . $field_name . '-field"]' => ['value' => '_submapping'],
                  'and',
                  ':input[id="edit-mapping-' . $field_name . '-submapping"]' => ['!value' => 'custom'],
                ],
              ],
            ];
            // There is an existing BrAPI datatype mapping, add link for editing.
            $mapping_form[$field_name]['other_submapping']['bmlink'] = [
              '#type' => 'link',
              '#title' => $this->t('Edit %brapi_mapping mapping', ['%brapi_mapping' => $other_mapping->getMappedEntityTypeId(),]),
              '#url' => Url::fromRoute('entity.brapidatatype.edit_form', ['brapidatatype' => $target_datatype_id]),
              '#attributes' => ['target' => ['_blank',],],
            ];
          }
          if (!empty($submapping)) {
            // There is a sub-mapping, add link for sub-mapping editing.
            $mapping_form[$field_name]['custom_submapping']['smlink'] = [
              '#type' => 'link',
              '#title' => $this->t('Edit custom sub-mapping'),
              '#url' => Url::fromRoute('entity.brapidatatype.edit_form', ['brapidatatype' => $submapping_datatype_id]),
              '#attributes' => ['target' => ['_blank',],],
            ];
          }
          else {
            // Add link to create sub-mapping.
            $mapping_form[$field_name]['custom_submapping']['smlink'] = [
              '#type' => 'link',
              '#title' => $this->t('Add custom sub-mapping'),
              '#url' => Url::fromRoute('entity.brapidatatype.add_form', ['mapping_id' => $submapping_datatype_id]),
              '#attributes' => ['target' => ['_blank',],],
            ];
          }

          // Select a referenced object id or current object.
          $mapping_form[$field_name]['subcontent'] = [
            '#type' => 'select',
            '#title' => $this->t('Select sub-content to map'),
            '#options' => $submapping_content_options,
            '#default_value' => $brapi_datatype->mapping[$field_name]['subcontent'] ?? '',
            '#empty_option' => $this->t('Current content'),
            '#empty_value' => '',
            '#attributes' => [
              'id' => 'edit-mapping-' . $field_name . '-subcontent',
            ],
            '#states' => [
              'visible' => [
                ':input[id="edit-mapping-' . $field_name . '-field"]' => ['value' => '_submapping'],
              ],
            ],
          ];

          // Finally sets field selection options.
          $mapping_form[$field_name]['field']['#options'] = $options;
        }

        // Custom value field.
        $mapping_form[$field_name]['custom'] = [
          '#type' => 'textfield',
          '#size' => '60',
          '#title' => $this->t('Custom value'),
          '#placeholder' => $this->t('Enter custom mapping'),
          '#default_value' => $brapi_datatype->mapping[$field_name]['custom'] ?? '',
          '#description' => $this->t(
            'The custom value can use both static text and <a href="https://goessner.net/articles/JsonPath/">JSON path</a>. JSON path are automatically detected and replaced by their corresponding field value(s). Note: JSON path filter and script expressions are not supported. A list of example is available <a href="#edit-json-path">here</a>.'
          ),
          '#states' => [
            'visible' => [
              [':input[id="edit-mapping-' . $field_name . '-field"]' => ['value' => '_custom']],
              'or',
              [
                ':input[id="edit-mapping-' . $field_name . '-field"]' => ['value' => '_submapping'],
                'and',
                ':input[id="edit-mapping-' . $field_name . '-subcontent"]' => ['value' => '_custom'],
              ],
            ],
          ],
        ];

        // JSON data.
        $mapping_form[$field_name]['is_json'] = [
          '#type' => 'checkbox',
          '#title' => $this->t('Treate custom value as a JSON structure'),
          '#default_value' => $brapi_datatype->mapping[$field_name]['is_json'] ?? FALSE,
          '#description' => $this->t(
            'If checked, the custom value will be parsed as JSON. Ex.: the custom value \'{"description": "$.title[0].value"}\' would be replace by a JSON object (with a "description" key which value would be a string containing current content title) rather than a string containing curly brackets arround some text (containing current content title as well).'
          ),
          '#states' => [
            'visible' => [
              ':input[id="edit-mapping-' . $field_name . '-field"]' => ['value' => '_custom'],
            ],
          ],
        ];
      }

      // Provide a list of available JSON Path with example values.
      try {
        try {
          // Try using bundle first.
          $bundle_key = \Drupal::entityTypeManager()->getDefinition($entity_type_id)->getKey('bundle');
          if (!empty($bundle_key)) {
            $last_number = max(\Drupal::entityQuery($entity_type_id)->condition($bundle_key, $bundle_id)->count()->execute() - 1, 0);
            $id = \Drupal::entityQuery($entity_type_id)->condition($bundle_key, $bundle_id)->range(rand(0, $last_number),1)->execute();
            if (!empty($id)) {
              $example_entity = \Drupal::entityTypeManager()->getStorage($entity_type_id)->load(current($id));
            }
          }
        }
        catch (\TypeError $e) {
          // Silently ignore bunlde failure and try an other way.
        }
        if (empty($last_number)) {
          // No bundle.
          $last_number = max(\Drupal::entityQuery($entity_type_id)->count()->execute() - 1, 0);
          $id = \Drupal::entityQuery($entity_type_id)->range(rand(0, $last_number),1)->execute();
          if (!empty($id)) {
            $example_entity = \Drupal::entityTypeManager()->getStorage($entity_type_id)->load(current($id));
          }
        }
        $mapping_form['json_path'] = [
          '#type' => 'details',
          '#title' => $this->t('JSON path info'),
          '#open' => FALSE,
          '#tree' => FALSE,
        ];
        if (!empty($example_entity)) {
          $definitions = $this->entityTypeManager->getDefinitions();
          $get_jpath = function ($data, $current_path = '$') use (&$get_jpath) {
            if (is_array($data)) {
              $jpath = [];
              if (array_key_exists(0, $data)) {
                $jpath = array_merge($jpath, $get_jpath($data[0], $current_path . '[0]'));
              }
              else {
                foreach ($data as $key => $value) {
                  $jpath = array_merge($jpath, $get_jpath($value, $current_path . '.' . $key));
                }
              }
              return $jpath;
            }
            else {
              return [$current_path . ' = "' . $data . '"'];
            }
          };

          $mapping_form['json_path']['examples'] = [
            '#type' => 'markup',
            '#markup' => $this->t(
              'List of example JSON Path for a random %entity:',
              ['%entity' => $definitions[$entity_type_id]->getLabel(),]
            )
            . '<br/><pre>'
            . implode(
              '<br/>',
              $get_jpath($example_entity->toArray())
            )
            . '</pre>'
          ];
        }
        else {
          $mapping_form['json_path']['examples'] = [
            '#type' => 'markup',
            '#markup' => $this->t('No example entity (%entity) available to provide JSON path examples.', ['%entity' => $definitions[$entity_type_id]->getLabel(),]),
          ];
        }
      }
      catch (\Throwable $e) {
        $this->logger('brapi')->error($e);
      }
    }

    return $mapping_form;
  }
}
