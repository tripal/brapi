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
                '#type' => 'fieldset',
                '#title' => $call,
              ];
              foreach ($call_definition['definition'] as $method => $method_def) {
                $missing_mappings = [];
                foreach ($call_definition['data_types'] ?? [] as $datatype => $enabled) {
                  // Skip special datatypes managed internally.
                  // Always allows the use of calls: v1/calls, v2/serverinfo.
                  if (
                    (('v2' == $version)
                      && (in_array($datatype,['WSMIMEDataTypes', 'ServerInfo']))
                    )
                    || (('v1' == $version)
                      && (in_array($datatype,['WSMIMEDataTypes', 'call']))
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
                }
                
                if (empty($missing_mappings)) {
                  $description = preg_replace(
                    "/\\n+/", "<br/>\n", $method_def['description']
                  );
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
                    '#default_value' => !empty($call_settings[$version][$call][$method]),
                  ];
                  $form[$version][$call][$method . '_datatypes'] = [
                    '#type' => 'markup',
                    '#markup' =>
                      $this->t("Used datatypes: %datatypes<br/>\n", ['%datatypes' => implode(', ', array_keys($call_definition['data_types']))])
                    ,
                  ];
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
