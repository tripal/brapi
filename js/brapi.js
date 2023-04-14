/**
 * @file
 * Contains BrAPI javascript code.
 */
(function ($, Drupal, drupalSettings) {

  'use strict';

  Drupal.brapi = {};

  /**
   * Attaches the JS test behavior to weight div.
   */
  Drupal.behaviors.brapi = {

    attach: function (context, settings) {
      var exportButton = document.getElementById('edit-brapi-mapping-export');
      if (!exportButton.classList.contains('brapi-button-processed')) {
        exportButton.classList.add('brapi-button-processed');
        exportButton.addEventListener('click', Drupal.brapi.exportMapping, false);
      }
      var importButton = document.getElementById('edit-brapi-mapping-import');
      if (!importButton.classList.contains('brapi-button-processed')) {
        importButton.classList.add('brapi-button-processed');
        importButton.addEventListener('click', Drupal.brapi.importMapping, false);
      }
    },
  };

  Drupal.brapi.exportMapping = function() {
    var brapiForm = document.getElementById("brapidatatype-edit-form");
    if (null !== brapiForm) {
      var data = new FormData(brapiForm);
      var settings = Object.fromEntries(data.entries());
      for (var key in settings) {
        if (("mapping" != key.substr(0, 7))
          && ("id" != key)
          && ("label" != key)
          && ("contentType" != key)
        ) {
          delete settings[key];
        }
      }
      var blob = new Blob([JSON.stringify(settings)], { type: 'text/plain' });
      var a = document.createElement('a');
      var mapid = data.get('id') ? data.get('id') + '_' : '';
      a.download = 'brapi_' + mapid + 'mapping.yml';
      a.href = window.URL.createObjectURL(blob);
      a.click();
    }
    else {
      alert("BrAPI datatype mapping form not found!");
    }
  };

  Drupal.brapi.LoadMapping = function(file) {
    var brapiForm = document.getElementById("brapidatatype-edit-form");
    if (null !== brapiForm) {
      var reader = new FileReader();
      reader.onload = function() {
        var text = reader.result;
        try {
          // @todo: We need to support YAML.
          // Maybe check if the lib https://github.com/nodeca/js-yaml is
          // available and fallback to JSON if not.
          var mapping = JSON.parse(text);
          for (var brapiElementName in mapping) {
            if ('mapping' === brapiElementName.substr(0, 7)) {
              var brapiElement = brapiForm.querySelector('[name="' + brapiElementName + '"]');
              if (null !== brapiElement) {
                if ("INPUT" === brapiElement.tagName
                  && (("checkbox" === brapiElement.attributes['type'])
                    || ("radio" === brapiElement.attributes['type']))
                ) {
                  if ('' === mapping[brapiElementName]) {
                    brapiElement.checked = false;
                  }
                  else {
                    brapiElement.checked = true;
                  }
                }
                else {
                  brapiElement.value = mapping[brapiElementName];
                }
              }
              else {
                console.log('BrAPI mapping field not found: ' + brapiElementName);
              }
            }
          }
        }
        catch(e) {
          console.log(e);
          alert('Unsupported format. Current implementation only supports JSON. ' + e);
        }
      };
      reader.readAsText(file);
    }
    else {
      alert("BrAPI datatype mapping form not found!");
    }
  };

  Drupal.brapi.importMapping = function() {
    var $brapiDialog = $(
      '<label>Select BrAPI mapping file: <input type="file" accept=".yml,.yaml,.json" /></label>'
    ).appendTo('body');
    Drupal.dialog($brapiDialog, {
      title: 'Load BrAPI Datatype Mapping',
      buttons: [
        {
          text: 'Load',
          click: function() {
            var input = event
              .target
              .closest('.ui-dialog')
              .querySelector('input[type="file"]')
            ;
            Drupal.brapi.LoadMapping(input.files[0]);
            $(this).dialog('close');
          },
        },
        {
          text: 'Cancel',
          click: function() {
            $(this).dialog('close');
          },
        },
      ],
    }).showModal();
  };

})(jQuery, Drupal, drupalSettings);

