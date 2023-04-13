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
      'ListDetails',
      'ListSummary',
      'ListValue',
    ];

    // Get BrAPI versions.
    $brapi_versions = brapi_available_versions();

    // @todo Allow the use of a URL parameter to manage a specific version
    // mapping, even if it is not enabled (for export for instance).

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
              /*&& empty($datatype_definition['as_field_in'])*/
          ) {
            // Skip datatypes not used in calls. // or other datatypes.
            // $this->logger('brapi')->notice('Skipping datatype "%datatype" (v%version) has it does not seem to be used.', ['%datatype' => $datatype, '%version' => $active_def]);
            continue 1;
          }
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

          // Generate datatype machine name.
          $datatype_id = brapi_generate_datatype_id($datatype, $version, $active_def);
          
          $form[$active_def][$datatype] = [
            '#type' => 'details',
            '#title' => $datatype,
            '#open' => FALSE,
          ];

          // Display mapping list.
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
            $form[$active_def][$datatype]['#open'] = TRUE;
          }
          // @todo: add sub-mapping list.

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

        // Sort datatypes.
        ksort($form[$active_def]);

        // Complete mapping.
        $complete_options = [];
        foreach ($brapi_versions as $version => $subversions) {
          $subversion_numbers = array_keys($subversions);
          $complete_options = array_merge($complete_options, array_combine($subversion_numbers, $subversion_numbers));
        }
        unset($complete_options[$active_def]);

        $form[$active_def]['complete'] = [
          '#type' => 'container',
          '#weight' => count($brapi_definition['data_types']) + 15,
          '#attributes' => [
            'class' => ['container-inline'],
          ],
        ];
        $form[$active_def]['complete']['complete_version'] = [
          '#type' => 'select',
          '#title' => $this->t('Complete %def mapping from version', ['%def' => $active_def]),
          '#weight' => 5,
          '#options' => $complete_options,
        ];
        $form[$active_def]['complete']['complete_submit'] = [
          '#type' => 'submit',
          '#value' => $this->t('Complete'),
          '#submit' => [[$this, 'submitCompleteForm']],
          '#weight' => 10,
        ];
        // Export mapping.
        $form[$active_def]['export'] = [
          '#type' => 'submit',
          '#value' => $this->t('Export %def mapping', ['%def' => $active_def]),
          '#submit' => [[$this, 'submitExportForm']],
          '#weight' => count($brapi_definition['data_types']) + 20,
        ];
        // Import mapping.
        $form[$active_def]['import'] = [
          '#type' => 'submit',
          '#value' => $this->t('Import mapping'),
          '#submit' => [[$this, 'submitImportForm']],
          '#weight' => count($brapi_definition['data_types']) + 25,
        ];
      }
    }
    
    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    
  }

  /**
   * {@inheritdoc}
   */
  public function submitCompleteForm(array &$form, FormStateInterface $form_state) {
    \Drupal::messenger()->addMessage('Complete not implemented yet.');
  }

  /**
   * {@inheritdoc}
   */
  public function submitExportForm(array &$form, FormStateInterface $form_state) {
    \Drupal::messenger()->addMessage('Export not implemented yet.');
  }

  /**
   * {@inheritdoc}
   */
  public function submitImportForm(array &$form, FormStateInterface $form_state) {
    \Drupal::messenger()->addMessage('Import not implemented yet.');
  }

}
