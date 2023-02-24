<?php

namespace Drupal\brapi\Entity;

use Drupal\brapi\BrapiTokenInterface;
use Drupal\Core\Entity\ContentEntityBase;
use Drupal\Core\Entity\EntityChangedTrait;
use Drupal\Core\Entity\EntityStorageInterface;
use Drupal\Core\Entity\EntityTypeInterface;
use Drupal\Core\Field\BaseFieldDefinition;
use Drupal\Core\Session\AccountInterface;
use Drupal\datetime\Plugin\Field\FieldType\DateTimeItem;

/**
 * Defines the BrAPI Token entity.
 *
 * Token expiration is a timestamp. If set to -1, the token never expires.
 *
 * @ContentEntityType(
 *   id = "brapi_token",
 *   label = @Translation("BrAPI Access Token entity"),
 *   base_table = "brapi_token",
 *   entity_keys = {
 *     "id" = "id",
 *     "uuid" = "uuid"
 *   }
 * )
 *
 */
class BrapiToken extends ContentEntityBase implements BrapiTokenInterface {

  use EntityChangedTrait;

  /**
   * {@inheritdoc}
   */
  public static function preCreate(
    EntityStorageInterface $storage_controller,
    array &$values
  ) {
    parent::preCreate($storage_controller, $values);

    $token = bin2hex(random_bytes(16));
    $config = \Drupal::config('brapi.settings');
    $maxlifetime =
      $config->get('token_default_lifetime')
      ?? BRAPI_DEFAULT_TOKEN_LIFETIME
    ;
    $expiration = time() + $maxlifetime;

    $values += [
      'user_id' => \Drupal::currentUser()->id(),
      'token' => $token,
      'expiration' => $expiration,
    ];
  }

  /**
   * {@inheritdoc}
   */
  public static function baseFieldDefinitions(
    EntityTypeInterface $entity_type
  ) {

    $fields['id'] = BaseFieldDefinition::create('integer')
      ->setLabel(t('ID'))
      ->setDescription(t('The ID of the BrAPI Token entity.'))
      ->setReadOnly(TRUE);

    $fields['uuid'] = BaseFieldDefinition::create('uuid')
      ->setLabel(t('UUID'))
      ->setDescription(t('The UUID of the BrAPI Token entity.'))
      ->setReadOnly(TRUE);

    $fields['token'] = BaseFieldDefinition::create('string')
      ->setLabel(t('Token'))
      ->setDescription(t('The access token.'))
      ->setSettings([
        'max_length' => 32,
        'text_processing' => 0,
      ])
    ;

    $fields['expiration'] = BaseFieldDefinition::create('datetime')
      ->setLabel(t('Expiration date'))
      ->setDescription(t('The expiration date of the token. The token will be invalid after that date and may be automatically deleted after that date.'))
      ->setSettings([
        'datetime_type' => DateTimeItem::DATETIME_TYPE_DATETIME,
      ])
      ->setDisplayOptions('view', [
        'label' => 'above',
        'type' => 'datetime_default',
        'settings' => [
          'format_type' => 'medium',
        ],
      ])
    ;

    $fields['user_id'] = BaseFieldDefinition::create('entity_reference')
      ->setLabel(t('User Name'))
      ->setDescription(t('The name of the associated user.'))
      ->setSetting('target_type', 'user')
      ->setSetting('handler', 'default')
      ->setDisplayOptions('view', [
        'label' => 'above',
        'type' => 'author',
        'weight' => -3,
      ])
      ->setDisplayOptions('form', [
        'type' => 'entity_reference_autocomplete',
        'settings' => [
          'match_operator' => 'CONTAINS',
          'match_limit' => 10,
          'size' => 60,
          'placeholder' => '',
        ],
        'weight' => -3,
      ])
      ->setDisplayConfigurable('form', TRUE)
      ->setDisplayConfigurable('view', TRUE)
    ;

    return $fields;
  }

  /**
   * Returns tokens for the given user.
   *
   * @param ?\Drupal\Core\Session\AccountInterface $user
   *   The user account to use. Default: current user.
   * @param bool $include_expired
   *   Includes expired tokens. Default: FALSE.
   * @return array
   *   An array of BrAPI token entity or an empty array if no token matches.
   */
  public static function getUserTokens(
    ?AccountInterface $user = NULL,
    bool $include_expired = FALSE
  ) :array {
    if (empty($user)) {
      $user = \Drupal::currentUser();
    }

    // Tries to get user current tokens.
    // Don't check access as the token could have been created by an admin.
    $token_storage = \Drupal::entityTypeManager()->getStorage('brapi_token');
    $query = $token_storage->getQuery()
      ->accessCheck(FALSE)
      ->condition('user_id', $user->id())
      ->sort('expiration', 'DESC');
    ;
    // Limit to valid token if not including expired tokens.
    if (!$include_expired) {
      $group = $query
        ->orConditionGroup()
        ->condition('expiration', time(), '>')
        ->condition('expiration', 0, '<')
      ;
      $query->condition($group);
    }
    $ids = $query->execute();
    $tokens = [];
    if (count($ids)) {
      // Load current token.
      $tokens = $token_storage->loadMultiple($ids);
    }

    return $tokens;
  }

  /**
   * Returns an active access token for the given user.
   *
   * This function either returns a current valid token for the given user or
   * creates a new one if expired.
   *
   * @param ?\Drupal\Core\Session\AccountInterface $user
   *   The user account to use. Default: current user.
   * @param bool $renew
   *   Renews the token. Default: FALSE.
   * @return \Drupal\brapi\BrapiTokenInterface
   *   A BrAPI token entity.
   */
  public static function getUserToken(
    ?AccountInterface $user = NULL,
    bool $renew = FALSE
  ) :BrapiTokenInterface {
    if (empty($user)) {
      $user = \Drupal::currentUser();
    }
    $tokens = static::getUserTokens($user, $renew);

    if (count($tokens)) {
      // Get latest token.
      $token = current($tokens);
      // Renew if needed.
      if ($renew) {
        $token->renew();
      }
    }

    if (empty($token)) {
      // Generate a new token.
      $token_storage = \Drupal::entityTypeManager()->getStorage('brapi_token');
      $token = $token_storage->create([
        'user_id' => $user->id(),
      ]);
      $token->save();
    }

    return $token;
  }

  /**
   * Remove all expired tokens.
   */
  public static function purgeExpiredTokens() {

    $token_storage = \Drupal::entityTypeManager()->getStorage('brapi_token');
    $ids = $token_storage->getQuery()
      ->accessCheck(FALSE)
      ->condition('expiration', time(), '<=')
      ->condition('expiration', 0, '>')
      ->execute()
    ;
    if (count($ids)) {
      // Load expired tokens.
      $tokens = $token_storage->loadMultiple($ids);
      // Delete them.
      $token_storage->delete($tokens);
    }

    return count($ids);
  }

  /**
   * {@inheritdoc}
   */
  public function isExpired() :bool {
    return
      ($this->expiration->value >= 0)
      && ($this->expiration->value < time())
    ;
  }

  /**
   * {@inheritdoc}
   */
  public function renew() {
    if (0 > $this->expiration->value) {
      // Do nothing for permanent tokens.
      return;
    }

    $config = \Drupal::config('brapi.settings');
    $maxlifetime =
      $config->get('token_default_lifetime')
      ?? BRAPI_DEFAULT_TOKEN_LIFETIME
    ;
    $expiration = time() + $maxlifetime;
    $this->expiration->setValue($expiration);
    $this->save();
  }

}
