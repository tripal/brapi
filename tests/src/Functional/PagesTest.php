<?php

namespace Drupal\Tests\brapi\Functional;

use Drupal\Core\Url;
use Drupal\Tests\BrowserTestBase;

/**
 * Simple test to ensure that main page loads with module enabled.
 *
 * @group brapi
 */
class PagesTest extends BrowserTestBase {

  /**
   * {@inheritdoc}
   */
  protected $defaultTheme = 'stark';

  /**
   * {@inheritdoc}
   */
  protected static $modules = [ 'brapi', ];

  /**
   * Fixture user with administrative powers.
   *
   * @var \Drupal\user\UserInterface
   */
  protected $adminUser;

  /**
   * Fixture authenticated user with no permissions.
   *
   * @var \Drupal\user\UserInterface
   */
  protected $authUser;

  /**
   * {@inheritdoc}
   */
  protected function setUp() :void {
    parent::setUp();
    $this->adminUser = $this->drupalCreateUser(['administer brapi']);
    $this->authUser = $this->drupalCreateUser([]);
  }

  /**
   * Tests anonymous BrAPI page access.
   */
  public function testAnonymousPages() {
    $this->drupalGet(Url::fromRoute('brapi.main'));
    $this->assertSession()->statusCodeEquals(200);

    $this->drupalGet(Url::fromRoute('brapi.documentation'));
    $this->assertSession()->statusCodeEquals(200);

    $this->drupalGet(Url::fromRoute('brapi.token'));
    $this->assertSession()->statusCodeEquals(403);
  }

  /**
   * Tests authenticated BrAPI page access.
   */
  public function testAuthenticatedPages() {
    // Authenticated user access.
    $this->drupalLogin($this->authUser);

    $this->drupalGet(Url::fromRoute('brapi.token'));
    $this->assertSession()->statusCodeEquals(200);

    $this->drupalGet(Url::fromRoute('brapi.admin'));
    $this->assertSession()->statusCodeEquals(403);

    $this->drupalGet(Url::fromRoute('brapi.datatypes'));
    $this->assertSession()->statusCodeEquals(403);

    $this->drupalGet(Url::fromRoute('entity.brapidatatype.list'));
    $this->assertSession()->statusCodeEquals(403);

    $this->drupalGet(Url::fromRoute('brapi.calls'));
    $this->assertSession()->statusCodeEquals(403);
  }

  /**
   * Tests admin BrAPI page access.
   */
  public function testAdminPages() {
    // Authenticated user access.
    $this->drupalLogin($this->authUser);

    $this->drupalGet(Url::fromRoute('brapi.token'));
    $this->assertSession()->statusCodeEquals(200);

    $this->drupalGet(Url::fromRoute('brapi.admin'));
    $this->assertSession()->statusCodeEquals(403);

    $this->drupalGet(Url::fromRoute('brapi.datatypes'));
    $this->assertSession()->statusCodeEquals(403);

    $this->drupalGet(Url::fromRoute('entity.brapidatatype.list'));
    $this->assertSession()->statusCodeEquals(403);

    $this->drupalGet(Url::fromRoute('brapi.calls'));
    $this->assertSession()->statusCodeEquals(403);
  }

}
