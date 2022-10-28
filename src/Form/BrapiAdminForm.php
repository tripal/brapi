<?php

namespace Drupal\brapi\Form;

use Drupal\Core\Form\FormBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Url;

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
    
    // Default page size.
    $form['page_size'] = [
      '#type' => 'number',
      '#title' => $this->t('Default number of result in pages'),
      '#default_value' => $config->get('page_size') ?: BRAPI_DEFAULT_PAGE_SIZE,
      '#required' => TRUE,
    ];

    // Server info.
    $sys_config = \Drupal::config('system.site');
    $form['server_info'] = [
      '#type'  => 'details',
      '#title' => $this->t('Server Info'),
      '#open'  => TRUE,
      '#tree'  => FALSE,
    ];
    $form['server_info']['server_name'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Server name'),
      '#default_value' => $config->get('server_name') ?? $sys_config->get('name'),
      '#size' => 60,
      '#maxlength' => 128,
    ];
    $form['server_info']['server_description'] = [
      '#type' => 'textarea',
      '#title' => $this->t('Server description'),
      '#default_value' => $config->get('server_name') ?? $sys_config->get('slogan'),
      '#size' => 60,
    ];
    $form['server_info']['contact_email'] = [
      '#type' => 'email',
      '#title' => $this->t('Contact e-mail'),
      '#default_value' => $config->get('contact_email') ?? $sys_config->get('mail'),
    ];
    $form['server_info']['documentation_url'] = [
      '#type' => 'url',
      '#title' => $this->t('Documentation URL'),
      '#default_value' =>
        $config->get('documentation_url')
        ?: Url::fromRoute('brapi.documentation', [], ['absolute' => TRUE])->toString()
      ,
    ];
    $form['server_info']['location'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Location'),
      '#default_value' => $config->get('location'),
      '#size' => 60,
      '#maxlength' => 128,
    ];
    $form['server_info']['organization_name'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Organisation name'),
      '#default_value' => $config->get('organization_name'),
      '#size' => 60,
      '#maxlength' => 128,
    ];
    $form['server_info']['organization_url'] = [
      '#type' => 'url',
      '#title' => $this->t('Organisation URL'),
      '#default_value' => $config->get('organization_url'),
    ];

    // Form save button.
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
    // Save version settings.
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
    // Save server settings.
    $config
      ->set('server_name', $form_state->getValue('server_name') ?? '')
      ->set('server_description', $form_state->getValue('server_description') ?? '')
      ->set('contact_email', $form_state->getValue('contact_email') ?? '')
      ->set('documentation_url', $form_state->getValue('documentation_url') ?? '')
      ->set('location', $form_state->getValue('location') ?? '')
      ->set('organization_name', $form_state->getValue('organization_name') ?? '')
      ->set('organization_url', $form_state->getValue('organization_url') ?? '')
    ;
    $config->save();
  }

}
