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
class BrapiAdminForm extends FormBase {

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'brapi_admin_form';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {

    // Get current settings.
    $config = \Drupal::config('brapi.settings');

    // Get BrAPI versions.
    $versions = brapi_available_versions();
    
    foreach ($versions as $version => $version_definition) {
      $version_options = array_keys($version_definition);
      $version_options = array_combine($version_options, $version_options);
      if (!empty($version_options)) {
        $form[$version] = [
          '#type' => 'checkbox',
          '#title' => $this->t(
            'Enable BrAPI %version endpoint',
            ['%version' => $version]
          ),
          '#return_value' => TRUE,
          '#default_value' => $config->get($version),
        ];
        $form[$version . '_definition'] = [
          '#type' => 'select',
          '#options' => $version_options,
          '#title' => $this->t('Select an implementation'),
          '#default_value' => $config->get($version . 'def'),
          '#states' => [
            'enabled' => [
              ':input[name="' . $version . '"]' => ['checked' => TRUE],
            ],
          ],
        ];
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
    $config = \Drupal::service('config.factory')->getEditable('brapi.settings');
    
    $versions = brapi_available_versions();
    foreach ($versions as $version => $version_definition) {
      $definition = $form_state->getValue($version . '_definition') ?? '';
      if (!empty($version_definition[$definition])) {
        $config
          ->set($version, $form_state->getValue($version) ?? FALSE)
          ->set($version . 'def', $definition)
        ;
      }
    }
    $config->save();
  }

}
