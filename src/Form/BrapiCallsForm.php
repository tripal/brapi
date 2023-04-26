<?php

namespace Drupal\brapi\Form;

use Drupal\Core\Entity\EntityStorageInterface;
use Drupal\Core\Form\FormBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\user\Entity\Role;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * A simple form that displays a select box and submit button.
 *
 * This form will be be themed by the 'theming_example_select_form' theme
 * handler.
 */
class BrapiCallsForm extends FormBase {

  /**
   * An entity query factory for the BrAPI Datatype Mapping entity type.
   *
   * @var \Drupal\Core\Entity\EntityStorageInterface
   */
  protected $entityStorage;

  /**
   * Constructs an BrapiDatatypeFormBase object.
   *
   * @param \Drupal\Core\Entity\EntityStorageInterface $entity_storage
   *   The entity type manager.
   */
  public function __construct(
    EntityStorageInterface $entity_storage
  ) {
    $this->entityStorage = $entity_storage;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    $form = new static(
      $container->get('entity_type.manager')->getStorage('brapidatatype'),
    );
    $form->setMessenger($container->get('messenger'));
    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'brapi_calls_form';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {

    // V1 internal types.
    $v1_internal_types = [
      'WSMIMEDataTypes',
      'call',
      'successfulSearchResponse_result',
    ];
    // V2 internal types.
    $v2_internal_types = [
      'WSMIMEDataTypes',
      'ContentTypes',
      'ServerInfo',
      'ListTypes',
    ];
    // Global query parameters (nb. "Authorization" *should* not be in query).
    $global_query_parameters = [
      'page', 'pageSize', 'Authorization',
    ];

    // Get BrAPI versions.
    $brapi_versions = brapi_available_versions();

    // Get roles with restricted BrAPI access.
    $role_storage = \Drupal::service('entity_type.manager')
      ->getStorage('user_role')
    ;
    $role_query = $role_storage->getQuery()
      ->accessCheck(FALSE)
      ->condition('permissions.*', BRAPI_PERMISSION_USE)
    ;
    $ids = $role_query->execute();
    $read_roles = $role_storage->loadMultiple($ids);
    $role_query = $role_storage->getQuery()
      ->accessCheck(FALSE)
      ->condition('permissions.*', BRAPI_PERMISSION_SPECIFIC)
    ;
    $ids = $role_query->execute();
    $restricted_roles = $role_storage->loadMultiple($ids);
    $role_query = $role_storage->getQuery()
      ->accessCheck(FALSE)
      ->condition('permissions.*', BRAPI_PERMISSION_EDIT)
    ;
    $ids = $role_query->execute();
    $write_roles = $role_storage->loadMultiple($ids);

    // Get current settings.
    $config = \Drupal::config('brapi.settings');
    $call_settings = $config->get('calls');
    $active_definitions = [];
    foreach ($brapi_versions as $version => $version_definitions) {
      if ($config->get($version)) {
        $active_definitions[$version] = $config->get($version . 'def');
      }
    }

    foreach ($active_definitions as $version => $active_def) {
      if (!empty($active_def) && !empty($brapi_versions[$version][$active_def])) {
        $form[$version] = [
          '#type' => 'details',
          '#title' => $this->t($version . ' (%def) call settings', ['%def' => $active_def]),
          '#open' => FALSE,
          '#tree' => TRUE,
        ];

        $brapi_definition = brapi_get_definition($version, $active_def);
        foreach ($brapi_definition['modules'] as $module => $categories) {
          foreach ($categories as $category => $elements) {
            if (is_numeric($category)) {
              // Skip category details.
              continue 1;
            }
            foreach (array_keys($elements['calls'] ?? []) as $call) {
              $call_definition = $brapi_definition['calls'][$call];
              $form[$version][$call] = [
                '#type' => 'details',
                '#title' => $call,
              ];
              $has_enabled_calls = FALSE;
              foreach ($call_definition['definition'] as $method => $method_def) {
                $missing_mappings = [];
                $mapped_datatypes = [];
                foreach ($call_definition['data_types'] ?? [] as $datatype => $enabled) {
                  // Skip special datatypes managed internally.
                  // Always allows the use of calls: v1/calls, v2/serverinfo,
                  // v1 search calls, v2/lists.
                  if (
                    (('v2' == $version)
                      && (in_array($datatype, $v2_internal_types))
                    )
                    || (('v1' == $version)
                      && (in_array($datatype, $v1_internal_types))
                    )
                  ) {
                    continue 1;
                  }
                  // Check if data type is mapped.
                  $mapping_id = brapi_generate_datatype_id($datatype, $version, $active_def);
                  $mapping = $this->entityStorage->load($mapping_id);
                  if (empty($mapping)) {
                    $missing_mappings[] = $datatype;
                  }
                  else {
                    $mapped_datatypes[] = $datatype;
                  }
                }

                if (empty($missing_mappings)) {
                  $description = preg_replace(
                    "/\\n+/", "<br/>\n", $method_def['description']
                  );
                  $call_enabled = !empty($call_settings[$version][$call][$method]);
                  if ($call_enabled) {
                    $has_enabled_calls = TRUE;
                  }
                  $form[$version][$call][$method] = [
                    '#type' => 'checkbox',
                    '#title' =>
                      '<b class="brapi-method brapi-method-'
                      . $method
                      . '">'
                      . strtoupper($method)
                      . '</b>: '
                      . $description
                    ,
                    '#default_value' => $call_enabled,
                  ];
                  // Adds auto-checking for deffered search.
                  $deferred_search_call = FALSE;
                  if (str_contains($call, 'searchResultsDbId')) {
                    $deferred_search_call = TRUE;
                    $parent_call_id = strtolower(str_replace(['/{searchResultsDbId}', '/'], '', $call));
                    $form[$version][$call][$method]['#states'] = [
                      'checked' => [
                        ':input[id="edit-' . $version . '-' . $parent_call_id . '-deferred"]' => ['checked' => TRUE,],
                      ],
                    ];
                  }
                  $form[$version][$call][$method . '_datatypes'] = [
                    '#type' => 'markup',
                    '#markup' =>
                      $this->t("Used datatypes: %datatypes<br/>\n", ['%datatypes' => implode(', ', array_keys($call_definition['data_types']))])
                    ,
                  ];

                  // Add deferred result option for search-* calls.
                  $search_call = FALSE;
                  if (str_starts_with($call, '/search')
                      && !str_contains($call, 'searchResultsDbId')
                  ) {
                    $search_call = TRUE;
                    $child_call_id = strtolower(str_replace(['/', '{', '}'], '', $call . 'searchresultsdbid-get'));
                    $form[$version][$call]['deferred'] = [
                      '#type' => 'checkbox',
                      '#title' => $this->t('Use background search (provide a "searchResultsDbId" and deferred results asynchronously)'),
                      '#default_value' => !empty($call_settings[$version][$call]['deferred']),
                      '#states' => [
                        'checked' => [
                          ':input[id="edit-' . $version . '-' . $child_call_id . '"]' => ['checked' => TRUE,],
                        ],
                      ],
                    ];
                  }

                  // Add role-specific permission settings.
                  $call_id = strtolower(
                    str_replace(['/', '{', '}'], '', $call) . '-' . $method
                  );
                  $form[$version][$call]['access'][$method] = [
                    '#type' => 'details',
                    '#title' => $this->t('Roles allowed to use the method'),
                    '#open' => FALSE,
                    '#tree' => TRUE,
                    '#attributes' => [
                      'class' => ['brapi-call-role-list'],
                    ],
                    '#states' => [
                      'visible' => [
                        ':input[id="edit-' . $version . '-' . $call_id . '"]' => ['checked' => TRUE,],
                      ],
                    ],
                  ];
                  if (!empty($restricted_roles)) {
                    $form[$version][$call]['access'][$method]['_label_'] = [
                      '#type' => 'markup',
                      '#markup' => $this->t('<div>Allow for:</div>'),
                    ];
                    foreach ($restricted_roles as $role) {
                      $form[$version][$call]['access'][$method][$role->id()] = [
                        '#type' => 'checkbox',
                        '#title' => $role->label(),
                        '#default_value' => !empty($call_settings[$version][$call]['access'][$method][$role->id()]),
                      ];
                    }
                  }
                  // Check method type and call type to determine permission
                  // type (read/write).
                  if (('get' == $method)
                    || $search_call
                    || $deferred_search_call
                  ) {
                    // Read access.
                    if (empty($restricted_roles)
                        && empty($write_roles)
                        && empty($read_roles)
                    ) {
                      // Nobody except admins.
                      $form[$version][$call]['access'][$method]['_default_'] = [
                        '#type' => 'markup',
                        '#markup' => $this->t('Administrators only'),
                      ];
                    }
                    else {
                      // List other roles with global permissions.
                      $all_read_roles = array_map(
                        function ($role) {
                          return $role->label();
                        },
                        array_merge($read_roles, $write_roles)
                      );
                      sort($all_read_roles);
                      $form[$version][$call]['access'][$method]['_default_'] = [
                        '#type' => 'markup',
                        '#markup' => $this->t(
                          'Roles allowed by global permissions: %roles',
                          ['%roles' => implode(', ', $all_read_roles)]
                        ),
                      ];
                    }
                  }
                  else {
                    // Write access.
                    if (empty($restricted_roles) && empty($write_roles)) {
                      // Nobody except admins.
                      $form[$version][$call]['access'][$method]['_default_'] = [
                        '#type' => 'markup',
                        '#markup' => $this->t('Administrators only'),
                      ];
                    }
                    else {
                      // List other roles with global permissions.
                      $all_write_roles = array_map(
                        function ($role) {
                          return $role->label();
                        },
                        $write_roles
                      );
                      sort($all_write_roles);
                      $form[$version][$call]['access'][$method]['_default_'] = [
                        '#type' => 'markup',
                        '#markup' => $this->t(
                          '<div>Roles allowed by global permissions: %roles</div>',
                          ['%roles' => implode(', ', $all_write_roles)]
                        ),
                      ];
                    }
                  }
                }
                else {
                  $form[$version][$call][$method] = [
                    '#type' => 'markup',
                    '#markup' =>
                      $this->t("Method %method: missing data type mapping for: %datatypes<br/>\n", ['%datatypes' => implode(', ', $missing_mappings), '%method' => strtoupper($method), ])
                    ,
                  ];
                  if (str_contains($call, 'searchResultsDbId')) {
                    $parent_call_id = strtolower(str_replace(['/{searchResultsDbId}', '/'], '', $call));
                  }
                }
              }

              // Open call section if one is senabled.
              if ($has_enabled_calls) {
                $form[$version][$call]['#open'] = TRUE;
              }
              // Conditional open for searchdDbId.
              if (str_contains($call, 'searchResultsDbId')) {
                $form[$version][$call]['#states']['expanded'] = [
                  ':input[id="edit-' . $version . '-' . $parent_call_id . '-deferred"]' => ['checked' => TRUE,],
                ];
              }
            }
          }
        }
      }
      ksort($form[$version]);
    }

    $form['submit'] = [
      '#type' => 'submit',
      '#value' => $this->t('Save'),
    ];
    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {

    // Get BrAPI versions.
    $brapi_versions = brapi_available_versions();

    // Get current settings.
    $config = \Drupal::service('config.factory')->getEditable('brapi.settings');
    $active_definitions = [];
    foreach ($brapi_versions as $version => $version_definitions) {
      if ($config->get($version)) {
        $active_definitions[$version] = $config->get($version . 'def');
      }
    }

    // Get the list of active calls.
    $calls = [];
    foreach ($active_definitions as $version => $active_def) {
      $call_values = $form_state->getValue($version);
      foreach ($call_values as $call => $call_settings) {
        foreach ($call_settings as $call_setting => $call_setting_value) {
          // $call_setting can be either a method ('post', 'get', etc.) or a
          // specific setting such as 'deferred'.
          if (!empty($call_setting_value)) {
            $calls[$version][$call][$call_setting] = $call_setting_value;
          }
        }
      }
    }
    $config->set('calls', $calls);

    $config->save();
    // Update routes.
    \Drupal::service("router.builder")->rebuild();
    $this->messenger()->addMessage($this->t('BrAPI call settings have been updated.'));
    $this->logger('brapi')->notice('BrAPI call settings have been updated.');
  }

}
