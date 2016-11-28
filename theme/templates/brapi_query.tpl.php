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
  <label><?php echo t('BrAPI Service URL:'); ?> <input id="brapi_query_url" name="brapi_query_url" type="text" value="<?php echo $local_brapi_url;?>"/></label><br/>
</form>

<div id="brapi_query_calls">
<h4>Calls</h4>
<?php
  foreach ($calls as $call_name => $call_structure) {
?>
  <form class="brapi-query" action="<?php echo $local_brapi_url;?>" method="post" accept-charset="UTF-8">
    <fieldset>
      <legend>Call "<strong class="brapi-call-name"><?php echo $call_name ?></strong>" </legend>
<?php
    if ($call_structure['arguments']) {
      echo t('Arguments:') . "<br/>\n<ol>\n";
      foreach ($call_structure['arguments'] as $arg_number => $argument) {
        echo "<li class=\"brapi-query-argument\"><label>" . $argument['name'] . ": <input name=\"" . $argument['name'] . "\" type=\"text\" value=\"\"/></label></li>\n";
      }
      echo "</ol>\n";
    }

    if ($call_structure['filters']) {
      echo t('Filters:') . "<br/>\n<ol>\n";
      foreach ($call_structure['filters'] as $filter_name => $value_type) {
        // Handles filter types.
        if (is_array($value_type)) {
          echo "<li class=\"brapi-query-filter\"><label>" . $filter_name . ": <select name=\"" . $filter_name . "\">\n";
          echo "  <option value=\"\" selected=\"selected\"></option>\n";
          foreach ($value_type as $value) {
            echo "  <option value=\"$value\">$value</option>\n";
          }
          echo"</select></label></li>\n";
        }
        else {
          switch ($value_type) {
            case 'string':
            case 'int':
              echo "<li class=\"brapi-query-filter\"><label>" . $filter_name . ": <input name=\"" . $filter_name . "\" type=\"text\" value=\"\"/></label></li>\n";
              break;

            case 'bool':
              echo "<li class=\"brapi-query-filter\"><label>" . $filter_name . ": <select name=\"" . $filter_name . "\">\n";
              echo "  <option value=\"1\">TRUE</option>\n";
              echo "  <option value=\"0\">FALSE</option>\n";
              echo"</select></label></li>\n";
              break;

            case 'date':
              echo "<li class=\"brapi-query-filter brapi-datepicker\"><label>" . $filter_name . ": <input name=\"" . $filter_name . "\" type=\"text\" value=\"\"/></label></li>\n";
              break;

            default:
              break;
          }
        }
      }
      echo "</ol>\n";
    }
?>
    <label class="barpi-query-string brapi-query-page">Page number: <input name="page" type="text" value="0"/></label><br/>
    <label class="barpi-query-string brapi-query-size">Results per page: <input name="pageSize" type="text" value="<?php echo BRAPI_DEFAULT_PAGE_SIZE; ?>"/></label><br/>
    <label class="barpi-query-string brapi-query-pretty"><input name="pretty" type="checkbox" value="1" checked="checked"/> Output pretty JSON</label><br/>
    <?php
      if (user_access('administer')) {
        ?>
    <label class="barpi-query-string brapi-query-debug"><input name="debug" type="checkbox" value="1"/> Enable debug mode</label><br/>
        <?php
      }
    ?>
    <input class="brapi-query-button" type="submit" value="<?php echo t('Run call'); ?>"/>
   </fieldset>
  </form>

<?php
  }
?>
</div>

<h4 id="brapi_query_result_title">Service response</h4>
<code id="brapi_query_results">
</code>
