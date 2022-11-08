Feature: Add a file to a page
  In order to add a file to a page
  As a site administrator
  I need to be able to use the file widget

  Background:
    Given I am logged in as a user with the "administrator" role
      And I am viewing a landing page
    When I customize this page with the Panels IPE
      And I click "Add new pane"
      And I click "Add file" in the "CTools modal" region
    Then I should see "Configure new Add file"

  @api @javascript @panopoly_widgets @local_files
  Scenario: Add a file
    When I fill in the following:
      | Title  | Testing file title |
      | Editor | plain_text         |
      | Text   | Testing file text  |
      And I click "Browse"
      And I switch to the frame "mediaBrowser"
    Then I should see "Allowed file types: txt doc docx xls xlsx pdf ppt pptx pps ppsx odt ods odp."
    When I attach the file "test.txt" to "files[upload]"
      And I press "Next"
    Then I should see "Destination"
    When I select the radio button "Public local files served by the webserver."
      And I press "Next"
      And I wait 2 seconds
      And I switch out of all frames
    Then I should see "test.txt"
    When I press "Add" in the "CTools modal" region
      And I press "Save"
      And I wait for the Panels IPE to deactivate
    Then I should see "Testing file title"
      And I should see "Testing file text"
      And I should see the link "test.txt"
