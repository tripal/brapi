<?php

namespace Drupal\brapi\Form;

use Drupal\Core\Entity\EntityConfirmFormBase;
use Drupal\Core\Url;
use Drupal\Core\Form\FormStateInterface;

/**
 * Class BrapiDatatypeDeleteForm.
 */
class BrapiDatatypeDeleteForm extends EntityConfirmFormBase {

  /**
   * {@inheritdoc}
   */
  public function getQuestion() {
    return $this->t('Are you sure you want to delete BrAPI Datatype mapping %label?', [
      '%label' => $this->entity->label(),
    ]);
  }

  /**
   * {@inheritdoc}
   */
  public function getConfirmText() {
    return $this->t('Delete BrAPI Datatype Mapping');
  }

  /**
   * {@inheritdoc}
   */
  public function getCancelUrl() {
    return new Url('brapi.datatypes');
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    // Delete the entity.
    $this->entity->delete();

    // Set a message that the entity was deleted.
    $this->messenger()->addMessage($this->t('BrAPI Datatype Mapping %label was deleted.', [
      '%label' => $this->entity->label(),
    ]));

    // Clear cache.
    \Drupal::cache('brapi_search')->invalidateAll();
    $this->messenger()->addMessage($this->t('BrAPI search cache has been cleared.'));

    // Redirect the user to the list controller when complete.
    $form_state->setRedirectUrl($this->getCancelUrl());
  }

}
