<?php

namespace Drupal\brapi\Form;

use Drupal\Core\Form\FormBase;
use Drupal\Core\Form\FormStateInterface;

/**
 * A simple form that displays a select box and submit button.
 *
 * This form will be be themed by the 'theming_example_select_form' theme
 * handler.
 */
class BrapiDataTypesForm extends FormBase {

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'brapi_datatypes_form';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {

    // Get BrAPI versions.
    $brapi_versions = brapi_available_versions();

    // Get current settings.
    $config = \Drupal::config('brapi.settings');
    $active_definitions = [];
    foreach ($brapi_versions as $version => $version_definition) {
      if ($config->get($version)) {
        $active_definitions[$version] = $config->get($version . 'def');
      }
    }
    
    // Get data type mapping entities.
    $mapping_loader = \Drupal::service('entity_type.manager')->getStorage('brapidatatype');

    foreach ($active_definitions as $version => $active_def) {
      if (!empty($active_def) && !empty($brapi_versions[$version][$active_def])) {
        $form[$active_def] = [
          '#type' => 'details',
          '#title' => $this->t($version . ' (%def) data type settings', ['%def' => $active_def]),
          '#open' => FALSE,
          '#tree' => TRUE,
        ];

        $brapi_definition = brapi_get_definition($version, $active_def);
        foreach ($brapi_definition['data_types'] as $datatype => $datatype_definition) {
          if (empty($datatype_definition['calls'])
              && empty($datatype_definition['as_field_in'])
          ) {
            // Skip datatypes not used in calls or other datatypes.
            $this->logger('brapi')->notice('Skipping datatype "%datatype" has it does not seem to be used.', ['%datatype' => $datatype, ]);
            continue 1;
          }

          // Generate datatype machine name.
          $datatype_id = brapi_generate_datatype_id($datatype, $version, $active_def);
          
          $form[$active_def][$datatype] = [
            '#type' => 'details',
            '#title' => $datatype,
            '#open' => FALSE,
          ];

          // @todo: display mapping status: unampped, mapped to content...
          $mapping = $mapping_loader->load($datatype_id);
          if (empty($mapping)) {
            $form[$active_def][$datatype]['action_link'] = [
              '#type' => 'link',
              '#title' => $this->t('Add mapping'),
              '#url' => \Drupal\Core\Url::fromRoute('entity.brapidatatype.add_form', ['mapping_id' => $datatype_id]),
            ];
          }
          else {
            $form[$active_def][$datatype]['action_link'] = [
              '#type' => 'link',
              '#title' => $this->t('Edit mapping'),
              '#url' => \Drupal\Core\Url::fromRoute('entity.brapidatatype.edit_form', ['brapidatatype' => $datatype_id]),
            ];
          }
          // @todo: display "Map content" button.
          $form[$active_def][$datatype]['details'] = [
            '#type' => 'markup',
            '#markup' =>
              '<br/>Id: '
              . $datatype_id
              . '<br/>Type: '
              . $datatype_definition['type']
              . '<br/>Calls :'
              . implode(', ', array_keys($datatype_definition['calls']))
              . '<br/>Fields: '
              . implode(', ', array_keys($datatype_definition['fields']))
            ,
          ];
        }
      }
    }

    // Sort datatypes.
    ksort($form[$active_def]);
    

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
  }

}
