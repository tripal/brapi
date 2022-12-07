<?php

namespace Drupal\brapi\Form;

use Drupal\Core\Entity\EntityStorageInterface;
use Drupal\Core\Form\FormBase;
use Drupal\Core\Form\FormStateInterface;
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
      'ServerInfo',
      'ListTypes',
      'ListSummary',
    ];
    // Global query parameters (nb. "Authorization" *should* not be in query).
    $global_query_parameters = [
      'page', 'pageSize', 'Authorization',
    ];
    
    // Get BrAPI versions.
    $brapi_versions = brapi_available_versions();

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
                  $form[$version][$call][$method . '_datatypes'] = [
                    '#type' => 'markup',
                    '#markup' =>
                      $this->t("Used datatypes: %datatypes<br/>\n", ['%datatypes' => implode(', ', array_keys($call_definition['data_types']))])
                    ,
                  ];

                  // Check if current method has query parameters.
                  $has_filtering_parameters = FALSE;
                  if (!empty($method_def['parameters'])) {
                    foreach ($method_def['parameters'] as $parameter) {
                      if (!empty($parameter['in'])
                          && ('query' == $parameter['in'])
                          && (!in_array($parameter['in'], $global_query_parameters))
                      ) {
                        $has_filtering_parameters = TRUE;
                      }
                    }
                  }
                  // Add filtering options for listing and search calls.
                  // Note: the radios could appear more than once by call if
                  // several methods allow filtering but it's unlikely to
                  // happen.
                  if ((!empty($mapped_datatypes) && $has_filtering_parameters)
                      || (str_starts_with($call, '/search'))
                  ) {
                    if (isset($call_settings[$version][$call]['filtering'])
                        && (in_array('' . $call_settings[$version][$call]['filtering'], ['drupal', 'brapi']))
                    ) {
                      $filtering_default = $call_settings[$version][$call]['filtering'];
                    }
                    else {
                      $filtering_default = 'drupal';
                    }
                    $form[$version][$call]['filtering'] = [
                      '#type' => 'radios',
                      '#title' => $this->t('Result filtering'),
                      '#default_value' => $filtering_default,
                      '#options' => [
                        'drupal' => $this->t('use Drupal entity filtering (recommended if functionnal)'),
                        'brapi'  => $this->t('use BrAPI module filtering (slower)'),
                      ],
                    ];
                  }
                  // Add deferred result option for search-* calls.
                  if (str_starts_with($call, '/search')
                      && !str_contains($call, 'searchResultsDbId')
                  ) {
                    $form[$version][$call]['deferred'] = [
                      '#type' => 'checkbox',
                      '#title' => $this->t('Use background search (provide a "searchResultsDbId" and deferred results asynchronously)'),
                      '#default_value' => !empty($call_settings[$version][$call]['deferred']),
                    ];
                  }
                }
                else {
                  $form[$version][$call][$method] = [
                    '#type' => 'markup',
                    '#markup' =>
                      $this->t("Method %method: missing data type mapping for: %datatypes<br/>\n", ['%datatypes' => implode(', ', $missing_mappings), '%method' => strtoupper($method), ])
                    ,
                  ];
                }
              }
              // Open call section if one is senabled.
              if ($has_enabled_calls) {
                $form[$version][$call]['#open'] = TRUE;
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
      foreach ($call_values as $call => $methods) {
        foreach ($methods as $method => $enabled) {
          if (!empty($enabled)) {
            $calls[$version][$call][$method] = TRUE;
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
