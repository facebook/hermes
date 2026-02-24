// Copyright (C) 2022 André Bargull. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.timezone
description: >
  TimeZone constructor accepts link names as its input.
features: [Temporal]
---*/

const testCases = [
  "America/Creston",  // Link America/Phoenix America/Creston
  "America/Nassau",  // Link America/Toronto America/Nassau
  "America/Atikokan",  // Link America/Panama America/Atikokan
  "America/Cayman",  // Link America/Panama America/Cayman
  "America/Anguilla",  // Link America/Puerto_Rico America/Anguilla
  "America/Antigua",  // Link America/Puerto_Rico America/Antigua
  "America/Aruba",  // Link America/Puerto_Rico America/Aruba
  "America/Curacao",  // Link America/Puerto_Rico America/Curacao
  "America/Blanc-Sablon",  // Link America/Puerto_Rico America/Blanc-Sablon   # Quebec (Lower North Shore)
  "America/Dominica",  // Link America/Puerto_Rico America/Dominica
  "America/Grenada",  // Link America/Puerto_Rico America/Grenada
  "America/Guadeloupe",  // Link America/Puerto_Rico America/Guadeloupe
  "America/Kralendijk",  // Link America/Puerto_Rico America/Kralendijk     # Caribbean Netherlands
  "America/Lower_Princes",  // Link America/Puerto_Rico America/Lower_Princes  # Sint Maarten
  "America/Marigot",  // Link America/Puerto_Rico America/Marigot        # St Martin (French part)
  "America/Montserrat",  // Link America/Puerto_Rico America/Montserrat
  "America/Port_of_Spain",  // Link America/Puerto_Rico America/Port_of_Spain  # Trinidad & Tobago
  "America/St_Barthelemy",  // Link America/Puerto_Rico America/St_Barthelemy  # St Barthélemy
  "America/St_Kitts",  // Link America/Puerto_Rico America/St_Kitts       # St Kitts & Nevis
  "America/St_Lucia",  // Link America/Puerto_Rico America/St_Lucia
  "America/St_Thomas",  // Link America/Puerto_Rico America/St_Thomas      # Virgin Islands (US)
  "America/St_Vincent",  // Link America/Puerto_Rico America/St_Vincent
  "America/Tortola",  // Link America/Puerto_Rico America/Tortola        # Virgin Islands (UK)
];

for (let id of testCases) {
  const tz = new Temporal.TimeZone(id);
  assert.sameValue(tz.id, id);
}
