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
    $brapi_versions = brapiAvailableVersions();

    // Get current settings.
    $config = \Drupal::config('brapi.settings');
    $active_definitions = [];
    foreach ($brapi_versions as $version => $version_definition) {
      if ($config->get($version)) {
        $active_definitions[$version] = $config->get($version . 'def');
      }
    }

    foreach ($active_definitions as $version => $active_def) {
      if (!empty($active_def) && !empty($brapi_versions[$version][$active_def])) {
        $form[$active_def] = [
          '#type' => 'fieldset',
          '#title' => $this->t($version . ' (%def) data type settings', ['%def' => $active_def]),
          '#tree' => TRUE,
        ];

        $brapi_definition = brapiGetDefinition($version, $active_def);
//\Drupal::messenger()->addMessage('DEBUG: ' . print_r($brapi_definition, TRUE)); //+debug
        foreach ($brapi_definition['modules'] as $module => $categories) {
          foreach ($categories as $category => $elements) {
            foreach (array_keys($elements['data_types']) as $datatype) {
              $datatype_definition = $brapi_definition['data_types'][$datatype];
              if (empty($datatype_definition['calls']) && empty($datatype_definition['as_field_in'])) {
                continue 1;
              }
              
              $form[$active_def][$datatype] = [
                '#type' => 'fieldset',
                '#title' => $datatype,
              ];
              $form[$active_def][$datatype]['test'] = [
                '#type' => 'markup',
                '#markup' =>
                  'type: '
                  . $datatype_definition['type']
                  . ', in calls ['
                  . implode(', ', array_keys($datatype_definition['calls']))
                  . ']'
                ,
              ];
            }
          }
        }
      }
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
  }

}
