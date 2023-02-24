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
    $brapi_datatype = $this->entity;

    // Build the form.
    if (!empty($mapping_id)) {
      $mapping_name = $mapping_id;
      if (preg_match(BRAPI_DATATYPE_ID_REGEXP, $mapping_id, $matches)) {
        list(, $version, $active_def, $datatype_name, $subfields) = $matches;
        $subfields = array_filter(explode('-', $subfields));
        // $brapi_definition = brapi_get_definition($version, $active_def);
        $mapping_name = $datatype_name . ' for BrAPI v' . $active_def;
      }
      if (!empty($subfields)) {
        $form['title'] = [
          '#type' => 'markup',
          '#markup' => $this->t('Data sub-mapping for BrAPI data type %datatype_name subfields %subfields', ['%datatype_name' => $datatype_name, '%subfields' => implode(', ', $subfields)]),
        ];
        $mapping_name = $datatype_name . '.' . implode('.', explode('-', $subfields)) . ' for BrAPI v' . $active_def;
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
      list($version, $active_def, $datatype_name, $subfields) = $brapi_datatype->parseId();
      if (!empty($subfields)) {
        $form['title'] = [
          '#type' => 'markup',
          '#markup' => $this->t('Data sub-mapping for BrAPI data type %datatype_name subfields %subfields', ['%datatype_name' => $datatype_name, '%subfields' => implode(', ', $subfields)]),
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
    }

    // Check for sub-mapping.
    if (!empty($subfields)) {
      // Get parent mapping datatype.
      $mapping_loader = \Drupal::service('entity_type.manager')
        ->getStorage('brapidatatype')
      ;
      $parent_datatype_id = substr($mapping_id, 0, strrpos($mapping_id, '-'));
      $parent_datatype = $mapping_loader->load($parent_datatype_id);
      $form['contentType'] = [
        '#type' => 'hidden',
        '#value' => $parent_datatype->contentType,
      ];
    }
    else
    {
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
    $form_state->setRedirect('brapi.datatypes');
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
    $string_field_options = [];
    $entityref_field_options = [];
    $static_field_option = ['_static' => '[Static value]',];
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
      // Loop on subfields.
      while (!empty($subfields)) {
        $subfield = array_shift($subfields);
        if (empty($brapi_definition['data_types'][$datatype_name]['fields'][$subfield])) {
          $this->logger('brapi')->error('Invalid sub-mapping identifier: %id. Stopping at sub-field "%field" of data type "%datatype_name".', ['%id' => $brapi_datatype->id(), '%field' => $subfield, '%datatype_name' => $datatype_name, ]);
          return [
            $field_name => [
              '#type' => 'markup',
              '#markup' => $this->t(
                'ERROR: Invalid sub-mapping identifier "@id"!',
                ['@id' => $brapi_datatype->id]
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
            'Map BrAPI field %field_name (%field_type) to field',
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
            + $static_field_option
          ;
        }
        else {
          // Complex types, objects and BrAPI datatypes.
          // Generate a select box for entity reference mapping and sub-mapping.
          $options =
            [
              '_submapping' => '[Sub-mapping]',
            ]
            + $string_field_options
            + $static_field_option
          ;
          // Offer sub-mapping.
          if ('object' != $base_datatype) {
            // Not a generic object, offer more options.

            // Check if a BrAPI datatype-specific mapping is available i Drupal entity fields.
            $target_datatype_id = brapi_generate_datatype_id($base_datatype, $version, $active_def);
            $mapping = $mapping_loader->load($target_datatype_id);
            if (!empty($mapping)) {
              // The datatype is mapped to a content type.
              // Check if we got an entity field that can reference that content type.
              if (!empty($entityref_field_options[$mapping->getMappedEntityTypeId()])) {
                // We have one or more field referencing that content type.
                $options =
                  ['_brapi_mapping' => '[Existing BrAPI-mapping]',]
                  + $options
                ;
                $mapping_form[$field_name]['brapi_mapping'] = [
                  '#type' => 'select',
                  '#title' => $this->t('Use existing BrAPI-mapped field'),
                  '#options' => 
                    $entityref_field_options[$mapping->getMappedEntityTypeId()],
                  '#default_value' => $brapi_datatype->mapping[$field_name]['brapi_mapping'] ?? '',
                  '#empty_value' => '',
                  '#states' => [
                    'visible' => [
                      ':input[id="edit-mapping-' . $field_name . '-field"]' => ['value' => '_brapi_mapping'],
                    ],
                  ],
                ];
              }
            }

            // Check if a sub-mnapping mapping is available.
            $submapping_datatype_id = $brapi_datatype->id . '-' . $field_name;
            $submapping = $mapping_loader->load($submapping_datatype_id);
            if (!empty($submapping)) {
              // There is a sub-mapping, add link for sub-mapping editing.
              // Can't use '#states' in  'link' element due to issue:
              // https://www.drupal.org/project/drupal/issues/2820586
              $mapping_form[$field_name]['submapping'] = [
                '#type' => 'fieldset',
                '#states' => [
                  'visible' => [
                    ':input[id="edit-mapping-' . $field_name . '-field"]' => ['value' => '_submapping'],
                  ],
                ],
              ];
              $mapping_form[$field_name]['submapping']['smlink'] = [
                '#type' => 'link',
                '#title' => $this->t('Edit sub-mapping'),
                '#url' => Url::fromRoute('entity.brapidatatype.edit_form', ['brapidatatype' => $submapping_datatype_id]),
                '#attributes' => ['target' => ['_blank',],],
              ];
            }
            else {
              // Add link to create sub-mapping.
              // Can't use '#states' in  'link' element due to issue:
              // https://www.drupal.org/project/drupal/issues/2820586
              $mapping_form[$field_name]['submapping'] = [
                '#type' => 'fieldset',
                '#states' => [
                  'visible' => [
                    ':input[id="edit-mapping-' . $field_name . '-field"]' => ['value' => '_submapping'],
                  ],
                ],
              ];
              $mapping_form[$field_name]['submapping']['smlink'] = [
                '#type' => 'link',
                '#title' => $this->t('Add sub-mapping'),
                '#url' => Url::fromRoute('entity.brapidatatype.add_form', ['mapping_id' => $submapping_datatype_id]),
                '#attributes' => ['target' => ['_blank',],],
              ];
            }
          }
          $mapping_form[$field_name]['field']['#options'] = $options;
        }

        // Static value field.
        $mapping_form[$field_name]['static'] = [
          '#type' => 'textfield',
          '#size' => '60',
          '#placeholder' => $this->t('Enter the static value to use'),
          '#default_value' => $brapi_datatype->mapping[$field_name]['static'] ?? '',
          '#states' => [
            'visible' => [
              ':input[id="edit-mapping-' . $field_name . '-field"]' => ['value' => '_static'],
            ],
          ],
        ];          

        // JSON data.
        $state_conditions = [
          ['value' => '_submapping'],
        ];
        foreach (array_keys($string_field_options['References']) as $ref_val) {
          $state_conditions[] = 'or';
          $state_conditions[] = ['value' => $ref_val];
        }
        $mapping_form[$field_name]['is_json'] = [
          '#type' => 'checkbox',
          '#title' => $this->t('Treate value as JSON data'),
          '#default_value' => $brapi_datatype->mapping[$field_name]['is_json'] ?? FALSE,
          '#states' => [
            'invisible' => [
              ':input[id="edit-mapping-' . $field_name . '-field"]' => $state_conditions,
            ],
          ],
        ];

        // Sub-mapping.
        $mapping_form[$field_name]['subfield'] = [
          '#type' => 'textfield',
          '#size' => '60',
          '#title' => $this->t('JSON Path sub-field specification (if needed)'),
          '#description' => $this->t('If the selected mapped field is an object, you can specify a <a href="https://goessner.net/articles/JsonPath/">JSON path</a> to select the sub-value(s) to use.'),
          '#default_value' => $brapi_datatype->mapping[$field_name]['subfield'] ?? '',
        ];          

      }
    }

    return $mapping_form;
  }
}