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
        list(, $version, $active_def, $datatype_name) = $matches;
        // $brapi_definition = brapi_get_definition($version, $active_def);
        $mapping_name = $datatype_name . ' for BrAPI v' . $active_def;
      }

      $form['title'] = [
        '#type' => 'markup',
        '#markup' => $this->t('Data Mapping for BrAPI Data Type %datatype_name', ['%datatype_name' => $datatype_name]),
      ];
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
      $form['label'] = [
        '#type' => 'textfield',
        '#title' => $this->t('Label'),
        '#maxlength' => 255,
        '#default_value' => $brapi_datatype->label(),
        '#required' => TRUE,
      ];
      $form['id'] = [
        '#type' => 'machine_name',
        '#title' => $this->t('Machine name'),
        '#default_value' => $brapi_datatype->id(),
        '#machine_name' => [
          'exists' => [$this, 'exists'],
          'replace_pattern' => '([^a-z0-9_\-\.]+)',
          'error' => 'The machine-readable name must be unique, and can only contain lowercase letters, numbers, underscores, dashes and dots.',
        ],
        '#disabled' => !$brapi_datatype->isNew(),
      ];
    }

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
    return (bool) $result;
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
    $string_field_options = [
      '_static' => $this->t('Static value'),
    ];
    $entityref_field_options = [];
    if (preg_match('/^(.+):(.+)$/', $content_type, $matches)) {
      list(, $entity_type_id, $bundle_id) = $matches;
      if ($entity_type_id && $bundle_id) {
        $fields = $this->entityFieldManager->getFieldDefinitions(
          $entity_type_id,
          $bundle_id
        );
        foreach ($fields as $field_id => $field) {
          if ($field->getType() == 'entity_reference') {
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
            $string_field_options[$field_id] =
              $field->getLabel() . ' (' . $field->getName() . ')'
            ;
          }
        }
      }
    }

    // Build BrAPI data type field list.
    if (preg_match(BRAPI_DATATYPE_ID_REGEXP, $brapi_datatype->id, $matches)
    ) {
      list(, $version, $active_def, $datatype_name) = $matches;
      $brapi_definition = brapi_get_definition($version, $active_def);
      $brapi_fields =
        $brapi_definition['data_types'][$datatype_name]['fields']
        ?? []
      ;
      foreach ($brapi_fields as $field_name => $field_def) {
        $field_type = $field_def['type'];
        $required = $field_def['required'];
        //@todo: check the number of values (ie. array versus single values).
        $base_datatype = $type = rtrim($field_type, '[]');
        if (in_array($base_datatype, ['string'])) {
          // Generate select box for field mapping.
          // @todo: allow the use of static values.
          $mapping_form[$field_name] = [
            '#type' => 'select',
            '#title' => $this->t(
              'Map BrAPI field %field_name (%field_type) to field',
              ['%field_name' => $field_name, '%field_type' => $field_type]
            ),
            '#options' => $string_field_options,
            '#default_value' => $brapi_datatype->mapping[$field_name] ?? '',
            '#required' => $required,
            '#empty_value' => '',
            '#attributes' => [
              'name' => $field_name,
            ],
          ];
          $mapping_form['_static_' . $field_name] = [
            '#type' => 'textfield',
            '#size' => '60',
            '#placeholder' => $this->t('Enter a static value'),
            '#states' => [
              'visible' => [
                ':input[name="' . $field_name . '"]' => ['value' => '_static'],
              ],
            ],
          ];          
        }
        else {
          // Generate select box for entity reference mapping if available...
          // Generate datatype machine name and fetch mapping.
          $target_datatype_id = brapi_generate_datatype_id($base_datatype, $version, $active_def);
          $mapping = $mapping_loader->load($target_datatype_id);
          if (!empty($mapping)) {
            // The datatype is mapped to a content type.
            // Check if we got a field that can reference that content type.
            if (array_key_exists($mapping->getMappedEntityTypeId(), $entityref_field_options)) {
              // We have one or more field referencing that content type.
              $options = [
                $this->t('References') =>
                  $entityref_field_options[$mapping->getMappedEntityTypeId()],
                $this->t('Local fields (turned into objects)') =>
                  $string_field_options,
              ];
              $mapping_form[$field_name] = [
                '#type' => 'select',
                '#title' => $this->t(
                  'Map BrAPI field %field_name to field (%field_type)',
                  ['%field_name' => $field_name, '%field_type' => $field_type]
                ),
                '#options' => $options,
                '#default_value' => $brapi_datatype->mapping[$field_name] ?? '',
                '#required' => $required,
                '#empty_value' => '',
              ];
            }
            else {
              // No field is referencing that content type.
              $mapping_form[$field_name] = [
                '#type' => 'select',
                '#title' => $this->t(
                  'Map BrAPI field %field_name to field (%field_type)',
                  ['%field_name' => $field_name, '%field_type' => $field_type]
                ),
                '#options' => [],
                '#empty_option' => $this->t(
                  'No related reference field available'
                ),
                '#default_value' => '',
                '#required' => $required,
                '#empty_value' => '',
                '#disabled' => TRUE,
              ];
            }
          }
          else {
            // There is no mapping for that datatype.
            if ('object' == $base_datatype) {
              // If it is "object", it is ok as the data structure is "free".
              $mapping_form[$field_name] = [
                '#type' => 'select',
                '#title' => $this->t(
                  'Map BrAPI field %field_name to field (%field_type)',
                  ['%field_name' => $field_name, '%field_type' => $field_type]
                ),
                '#options' => [
                  $this->t('References') =>
                    $entityref_field_options['object'],
                  $this->t('Local fields (turned into objects)') =>
                    $string_field_options,
                ],
                '#default_value' => $brapi_datatype->mapping[$field_name] ?? '',
                '#required' => $required,
                '#empty_value' => '',
              ];
            }
            else {
              // Otherwise, no field can be used.
              $mapping_form[$field_name] = [
                '#type' => 'select',
                '#title' => $this->t(
                  'Map BrAPI field %field_name to field (%field_type)',
                  ['%field_name' => $field_name, '%field_type' => $field_type]
                ),
                '#options' => [],
                '#empty_option' => $this->t(
                  'BrAPI data type "%base_datatype" currently not mapped',
                  ['%base_datatype' => $base_datatype]
                ),
                '#default_value' => '',
                '#required' => $required,
                '#empty_value' => '',
                '#disabled' => TRUE,
              ];
            }
          }
        }
      }
    }

    return $mapping_form;
  }

}
