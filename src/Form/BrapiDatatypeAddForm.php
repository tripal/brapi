<?php

namespace Drupal\brapi\Form;

use Drupal\Core\Form\FormStateInterface;

/**
 * Class BrapiDatatypeAddForm.
 *
 * Provides the add form for our BrAPI Datatype Mapping entity.
 */
class BrapiDatatypeAddForm extends BrapiDatatypeFormBase {

  /**
   * {@inheritdoc}
   */
  public function validateForm(array &$form, FormStateInterface $form_state) {
    // @todo: Make sure the identifier does not already exist.
    parent::validateForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  protected function actions(array $form, FormStateInterface $form_state) {
    $actions = parent::actions($form, $form_state);
    $actions['submit']['#value'] = $this->t('Create a BrAPI Datatype Mapping');
    return $actions;
  }

}
