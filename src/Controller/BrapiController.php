<?php

/**
 * @file
 */

namespace Drupal\brapi\Controller;

use Drupal\Core\Controller\ControllerBase;
use Symfony\Component\HttpFoundation\JsonResponse;
use Drupal\Core\Url;

class BrapiController extends ControllerBase {

  /**
   * Initial landing page explaining the use of the module.
   *
   * We create a render array and specify the theme to be used through the use
   * of #theme_wrappers. With all output, we aim to leave the content as a
   * render array just as long as possible, so that other modules (or the theme)
   * can alter it.
   *
   * @see render_example.module
   * @see form_example_elements.inc
   */
  public function mainPage() {
    // $links = [];
    // $links[] = [
    //   '#type' => 'link',
    //   '#url' => Url::fromRoute('theming_example.form_text'),
    //   '#title' => t('Simple form 2'),
    // ];
    // $content = [
    //   '#theme' => 'item_list',
    //   '#theme_wrappers' => ['theming_example_content_array'],
    //   '#items' => $links,
    //   '#title' => t('Some examples of pages and forms that are run through theme functions.'),
    // ];
    $content = [
      '#theme' => 'brapi_main',
      '#title' => t('BrAPI Endpoint Details.'),
    ];

    return $content;
  }

  /**
   * Export test as JSON.
   */
  public function json() {
    $json_array = [
      'result' => [
        'something' => 1,
        'test' => ['a', 51, ],
      ],
    ];
    return new JsonResponse($json_array);
  }

}
