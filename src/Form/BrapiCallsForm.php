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
class BrapiCallsForm extends FormBase {

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
    $brapi_versions = brapiAvailableVersions();

    // Get current settings.
    $config = \Drupal::config('brapi.settings');
    $active_definitions = [];
    foreach ($brapi_versions as $version => $version_definitions) {
      if ($config->get($version)) {
        $active_definitions[$version] = $config->get($version . 'def');
      }
    }

    foreach ($active_definitions as $version => $active_def) {
      if (!empty($active_def) && !empty($brapi_versions[$version][$active_def])) {
        $form[$active_def] = [
          '#type' => 'fieldset',
          '#title' => $this->t($version . ' (%def) call settings', ['%def' => $active_def]),
          '#tree' => TRUE,
        ];

        $brapi_definition = brapiGetDefinition($version, $active_def);
        foreach ($brapi_definition['modules'] as $module => $categories) {
          foreach ($categories as $category => $elements) {
            foreach (array_keys($elements['calls']) as $call) {
              $call_definition = $brapi_definition['calls'][$call];
              $form[$active_def][$call] = [
                '#type' => 'fieldset',
                '#title' => $call,
              ];
              foreach ($call_definition['definition'] as $method => $method_def) {
                $form[$active_def][$call][$method] = [
                  '#type' => 'checkbox',
                  '#title' =>
                    '<b class="brapi-method brapi-method-'
                    . $method
                    . '">'
                    . strtoupper($method)
                    . '</b>: '
                    . $method_def['description']
                  ,
                ];
              }
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
    /*
      Array
      (
        [2.0] => Array
          (
            [/commoncropnames] => Array
              (
                [get] => 0
              )

            [/lists] => Array
              (
                [get] => 0
                [post] => 0
              )

            [/lists/{listDbId}] => Array
              (
                [get] => 0
                [put] => 0
              )

            [/lists/{listDbId}/items] => Array
              (
                [post] => 0
              )

            [/search/lists] => Array
              (
                [post] => 0
              )

            [/search/lists/{searchResultsDbId}] => Array
              (
                [get] => 0
              )

            [/locations] => Array
              (
                [get] => 0
                [post] => 0
              )

            [/locations/{locationDbId}] => Array
              (
                [get] => 0
                [put] => 0
              )

            [/search/locations] => Array
              (
                [post] => 0
              )

            [/search/locations/{searchResultsDbId}] => Array
              (
                [get] => 0
              )

            [/people] => Array
              (
                [get] => 0
                [post] => 0
              )

            [/people/{personDbId}] => Array
              (
                [get] => 0
                [put] => 0
              )

            [/search/people] => Array
              (
                [post] => 0
              )

            [/search/people/{searchResultsDbId}] => Array
              (
                [get] => 0
              )

            [/programs] => Array
              (
                [get] => 0
                [post] => 0
              )

            [/programs/{programDbId}] => Array
              (
                [get] => 0
                [put] => 0
              )

            [/search/programs] => Array
              (
                [post] => 0
              )

            [/search/programs/{searchResultsDbId}] => Array
              (
                [get] => 0
              )

            [/seasons] => Array
              (
                [get] => 0
                [post] => 0
              )

            [/seasons/{seasonDbId}] => Array
              (
                [get] => 0
                [put] => 0
              )

            [/serverinfo] => Array
              (
                [get] => 0
              )

            [/search/studies] => Array
              (
                [post] => 0
              )

            [/search/studies/{searchResultsDbId}] => Array
              (
                [get] => 0
              )

            [/studies] => Array
              (
                [get] => 0
                [post] => 0
              )

            [/studies/{studyDbId}] => Array
              (
                [get] => 0
                [put] => 0
              )

            [/studytypes] => Array
              (
                [get] => 0
              )

            [/search/trials] => Array
              (
                [post] => 0
              )

            [/search/trials/{searchResultsDbId}] => Array
              (
                [get] => 0
              )

            [/trials] => Array
              (
                [get] => 0
                [post] => 0
              )

            [/trials/{trialDbId}] => Array
              (
                [get] => 0
                [put] => 0
              )

            [/callsets] => Array
              (
                [get] => 0
              )

            [/callsets/{callSetDbId}] => Array
              (
                [get] => 0
              )

            [/callsets/{callSetDbId}/calls] => Array
              (
                [get] => 0
              )

            [/search/callsets] => Array
              (
                [post] => 0
              )

            [/search/callsets/{searchResultsDbId}] => Array
              (
                [get] => 0
              )

            [/calls] => Array
              (
                [get] => 0
              )

            [/search/calls] => Array
              (
                [post] => 0
              )

            [/search/calls/{searchResultsDbId}] => Array
              (
                [get] => 0
              )

            [/maps] => Array
              (
                [get] => 0
              )

            [/maps/{mapDbId}] => Array
              (
                [get] => 0
              )

            [/maps/{mapDbId}/linkagegroups] => Array
              (
                [get] => 0
              )

            [/markerpositions] => Array
              (
                [get] => 0
              )

            [/search/markerpositions] => Array
              (
                [post] => 0
              )

            [/search/markerpositions/{searchResultsDbId}] => Array
              (
                [get] => 0
              )

            [/referencesets] => Array
              (
                [get] => 0
              )

            [/referencesets/{referenceSetDbId}] => Array
              (
                [get] => 0
              )

            [/search/referencesets] => Array
              (
                [post] => 0
              )

            [/search/referencesets/{searchResultsDbId}] => Array
              (
                [get] => 0
              )

            [/references] => Array
              (
                [get] => 0
              )

            [/references/{referenceDbId}] => Array
              (
                [get] => 0
              )

            [/references/{referenceDbId}/bases] => Array
              (
                [get] => 0
              )

            [/search/references] => Array
              (
                [post] => 0
              )

            [/search/references/{searchResultsDbId}] => Array
              (
                [get] => 0
              )

            [/samples] => Array
              (
                [get] => 0
                [post] => 0
              )

            [/samples/{sampleDbId}] => Array
              (
                [get] => 0
                [put] => 0
              )

            [/search/samples] => Array
              (
                [post] => 0
              )

            [/search/samples/{searchResultsDbId}] => Array
              (
                [get] => 0
              )

            [/search/variantsets] => Array
              (
                [post] => 0
              )

            [/search/variantsets/{searchResultsDbId}] => Array
              (
                [get] => 0
              )

            [/variantsets] => Array
              (
                [get] => 0
              )

            [/variantsets/extract] => Array
              (
                [post] => 0
              )

            [/variantsets/{variantSetDbId}] => Array
              (
                [get] => 0
              )

            [/variantsets/{variantSetDbId}/calls] => Array
              (
                [get] => 0
              )

            [/variantsets/{variantSetDbId}/callsets] => Array
              (
                [get] => 0
              )

            [/variantsets/{variantSetDbId}/variants] => Array
              (
                [get] => 0
              )

            [/search/variants] => Array
              (
                [post] => 0
              )

            [/search/variants/{searchResultsDbId}] => Array
              (
                [get] => 0
              )

            [/variants] => Array
              (
                [get] => 0
              )

            [/variants/{variantDbId}] => Array
              (
                [get] => 0
              )

            [/variants/{variantDbId}/calls] => Array
              (
                [get] => 0
              )

            [/vendor/orders] => Array
              (
                [get] => 0
                [post] => 0
              )

            [/vendor/orders/{orderId}/plates] => Array
              (
                [get] => 0
              )

            [/vendor/orders/{orderId}/results] => Array
              (
                [get] => 0
              )

            [/vendor/orders/{orderId}/status] => Array
              (
                [get] => 0
              )

            [/vendor/plates] => Array
              (
                [post] => 0
              )

            [/vendor/plates/{submissionId}] => Array
              (
                [get] => 0
              )

            [/vendor/specifications] => Array
              (
                [get] => 0
              )

            [/crosses] => Array
              (
                [get] => 0
                [post] => 0
                [put] => 0
              )

            [/plannedcrosses] => Array
              (
                [get] => 0
                [post] => 0
                [put] => 0
              )

            [/crossingprojects] => Array
              (
                [get] => 0
                [post] => 0
              )

            [/crossingprojects/{crossingProjectDbId}] => Array
              (
                [get] => 0
                [put] => 0
              )

            [/breedingmethods] => Array
              (
                [get] => 0
              )

            [/breedingmethods/{breedingMethodDbId}] => Array
              (
                [get] => 0
              )

            [/germplasm] => Array
              (
                [get] => 0
                [post] => 0
              )

            [/germplasm/{germplasmDbId}] => Array
              (
                [get] => 0
                [put] => 0
              )

            [/germplasm/{germplasmDbId}/mcpd] => Array
              (
                [get] => 0
              )

            [/germplasm/{germplasmDbId}/pedigree] => Array
              (
                [get] => 0
              )

            [/germplasm/{germplasmDbId}/progeny] => Array
              (
                [get] => 0
              )

            [/search/germplasm] => Array
              (
                [post] => 0
              )

            [/search/germplasm/{searchResultsDbId}] => Array
              (
                [get] => 0
              )

            [/attributevalues] => Array
              (
                [get] => 0
                [post] => 0
              )

            [/attributevalues/{attributeValueDbId}] => Array
              (
                [get] => 0
                [put] => 0
              )

            [/search/attributevalues] => Array
              (
                [post] => 0
              )

            [/search/attributevalues/{searchResultsDbId}] => Array
              (
                [get] => 0
              )

            [/attributes] => Array
              (
                [get] => 0
                [post] => 0
              )

            [/attributes/categories] => Array
              (
                [get] => 0
              )

            [/attributes/{attributeDbId}] => Array
              (
                [get] => 0
                [put] => 0
              )

            [/search/attributes] => Array
              (
                [post] => 0
              )

            [/search/attributes/{searchResultsDbId}] => Array
              (
                [get] => 0
              )

            [/seedlots] => Array
              (
                [get] => 0
                [post] => 0
              )

            [/seedlots/transactions] => Array
              (
                [get] => 0
                [post] => 0
              )

            [/seedlots/{seedLotDbId}] => Array
              (
                [get] => 0
                [put] => 0
              )

            [/seedlots/{seedLotDbId}/transactions] => Array
              (
                [get] => 0
              )

            [/events] => Array
              (
                [get] => 0
              )

            [/images] => Array
              (
                [get] => 0
                [post] => 0
              )

            [/images/{imageDbId}] => Array
              (
                [get] => 0
                [put] => 0
              )

            [/images/{imageDbId}/imagecontent] => Array
              (
                [put] => 0
              )

            [/search/images] => Array
              (
                [post] => 0
              )

            [/search/images/{searchResultsDbId}] => Array
              (
                [get] => 0
              )

            [/methods] => Array
              (
                [get] => 0
                [post] => 0
              )

            [/methods/{methodDbId}] => Array
              (
                [get] => 0
                [put] => 0
              )

            [/observationlevels] => Array
              (
                [get] => 0
              )

            [/observationunits] => Array
              (
                [get] => 0
                [post] => 0
                [put] => 0
              )

            [/observationunits/table] => Array
              (
                [get] => 0
              )

            [/observationunits/{observationUnitDbId}] => Array
              (
                [get] => 0
                [put] => 0
              )

            [/search/observationunits] => Array
              (
                [post] => 0
              )

            [/search/observationunits/{searchResultsDbId}] => Array
              (
                [get] => 0
              )

            [/search/variables] => Array
              (
                [post] => 0
              )

            [/search/variables/{searchResultsDbId}] => Array
              (
                [get] => 0
              )

            [/variables] => Array
              (
                [get] => 0
                [post] => 0
              )

            [/variables/{observationVariableDbId}] => Array
              (
                [get] => 0
                [put] => 0
              )

            [/observations] => Array
              (
                [get] => 0
                [post] => 0
                [put] => 0
              )

            [/observations/table] => Array
              (
                [get] => 0
              )

            [/observations/{observationDbId}] => Array
              (
                [get] => 0
                [put] => 0
              )

            [/search/observations] => Array
              (
                [post] => 0
              )

            [/search/observations/{searchResultsDbId}] => Array
              (
                [get] => 0
              )

            [/ontologies] => Array
              (
                [get] => 0
              )

            [/scales] => Array
              (
                [get] => 0
                [post] => 0
              )

            [/scales/{scaleDbId}] => Array
              (
                [get] => 0
                [put] => 0
              )

            [/traits] => Array
              (
                [get] => 0
                [post] => 0
              )

            [/traits/{traitDbId}] => Array
              (
                [get] => 0
                [put] => 0
              )

          )

        [submit] => Drupal\Core\StringTranslation\TranslatableMarkup Object
          ...
        [form_build_id] => form-N_iQxrMyvffZgK0QRR8MtzDkn3CPA1Uct5iJGYObHmo
        [form_token] => gP7jfEqtfg76ofJqzbMhZfRu_MKC0wv9ApQ8CFpeATA
        [form_id] => brapi_calls_form
        [op] => Drupal\Core\StringTranslation\TranslatableMarkup Object
          ...
      )
    */
  }

}
