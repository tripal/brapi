<?php

/**
 * @file
 * Breeding API query interface page.
 *
 * @ingroup brapi
 */
?>

<h3>Breeding API Query Interface</h3>

<form id="brapi_query_settings">
  <label><?php echo t('BrAPI Service URL:'); ?> <input id="brapi_query_url" name="brapi_query_url" type="text" value="<?php echo $local_brapi_url;?>"/></label>
  <?php
  if (1 < count($brapi_sites)) {
  ?>
  <div class="container-inline">
    <label for="brapi_site_urls"><?php echo t('Use available sites:'); ?></label>
    <select id="brapi_site_urls" name="brapi_site">
      <?php
      $selected = ' selected="selected"';
      foreach ($brapi_sites as $brapi_site_name => $brapi_url) {
        ?>
        <option value="<?php echo check_url($brapi_url); ?>"<?php echo $selected; ?>><?php echo check_plain($brapi_site_name); ?></option>
        <?php
        $selected = '';
      }
      ?>
    </select>

  </div>
  <?php
  }
  ?>
  <div class="container-inline">
    <label for="brapi_api_version"><?php echo t('API Version:'); ?></label>
    <select id="brapi_api_version" name="brapi_api_version">
      <?php
        foreach (brapi_get_versions() as $api_version_mn => $api_version_name) {
          echo '      <option value="'
            . $api_version_mn
            . '"'
            . ((BRAPI_SERVICE_VERSION == $api_version_name)
              ? ' selected="selected"'
              : '')
            . '>'
            . $api_version_name
            . "</option>\n";
        }
      ?>
    </select>
  </div>

</form>

<div id="brapi_query_calls">
  <h4>Calls</h4>
  <form id="brapi_call_settings">
    <label for="brapi_call_select">Select a BrAPI call:</label>
    <select id="brapi_call_select" name="brapi_call_select">
    </select>
  </form>
  <?php
    // Render each call form.
    foreach ($calls as $call_name => $call_info) {
      $brapi_query = drupal_get_form(
        'brapi_query_form',
        $call_name,
        $call_info
      );
      print render($brapi_query);
    }
  ?>
</div>

<div id="brapi_query_results">
  <h4 id="brapi_query_result_title">Service response</h4>
  <iframe id="brapi_query_result_iframe" name="brapi_query_result_iframe"></iframe>
  <code id="brapi_query_result_ajax">
  </code>
</div>
