# Geriatric AI Assistant

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.19392781.svg)](https://doi.org/10.5281/zenodo.19392781)
[![License](https://img.shields.io/badge/License-Proprietary-red.svg)](LICENSE)
[![ES](https://img.shields.io/badge/Spanish-version-green.svg)](./README.md)

**An in-ear device that speaks with your family's voice. For elderly people with Alzheimer's, dementia, diabetic retinopathy, and type 2 diabetes.**

It is not a hearing aid. It is a companion.

## What problem does it solve?

An elderly person with Alzheimer's cannot read the date on their medication. They don't remember if they already ate. They wake up at 3 AM and don't know what to do. They feel alone, even when people are nearby. Family cannot be there 24/7.

**Geriatric AI Assistant solves that.**

## What does it do?

- **Reads** labels, expiration dates, medication instructions out loud.
- **Monitors** glucose (FreeStyle Libre 2) and alerts if there is a risk of hypoglycemia.
- **Detects** falls and issues a voice alert.
- **Remembers** recipes, routines, and recent conversations.
- **Time orientation**: "It's 3 AM. It's nighttime. Better go back to sleep."
- **Recognizes** people: "That's your son Juan. He came to visit you."
- **Virtual geofence**: if they leave home, it alerts and asks if they want to notify someone.
- **Conversational buffer**: "What was I saying?" → "You wanted flan."

## Why is it different?

- **It speaks with the voice your family chooses.** Not a robotic voice. The voice of their son, daughter, grandchild.
- **No internet needed.** It works offline. No data sent to the cloud.
- **No complicated setup.** Family sets it up once.
- **Costs less than 100 USD.** Existing components, no expensive licenses.

## Who is it for?

- People with Alzheimer's or senile dementia.
- Elderly people with diabetic retinopathy (difficulty reading).
- Patients with type 2 diabetes needing continuous monitoring.
- Families caring for a loved one.
- Nursing homes.

## Technology (all existing, all certified)

| Component | Price | Function |
|-----------|-------|----------|
| OV2640 camera | USD 5 | Read labels, recognize faces |
| MPU-6050 gyroscope | USD 2 | Detect falls |
| NFC reader | USD 10 | Connect to FreeStyle Libre 2 |
| ESP32 / nRF53 | USD 5-8 | Local processing |
| Battery | USD 3 | 7-day autonomy |
| Mini speaker | USD 2 | Clear, warm voice |

**Estimated total cost: USD 85**

## Current status

- Prototype in evolution.
- Uses existing certified equipment.
- No additional validation required.
- Complete documentation in the repository.

## License

Copyright © 2026 Enrique Aguayo. All rights reserved.


**An in-ear device that speaks with your family's voice. For elderly people with Alzheimer's, dementia, diabetic retinopathy, and type 2 diabetes.**

It is not a hearing aid. It is a companion.

## What problem does it solve?

An elderly person with Alzheimer's cannot read the date on their medication. They don't remember if they already ate. They wake up at 3 AM and don't know what to do. They feel alone, even when people are nearby. Family cannot be there 24/7.

**Geriatric AI Assistant solves that.**

## What does it do?

- **Reads** labels, expiration dates, medication instructions out loud.
- **Monitors** glucose (FreeStyle Libre 2) and alerts if there is a risk of hypoglycemia.
- **Detects** falls and issues a voice alert.
- **Remembers** recipes, routines, and recent conversations.
- **Time orientation**: "It's 3 AM. It's nighttime. Better go back to sleep."
- **Recognizes** people: "That's your son Juan. He came to visit you."
- **Virtual geofence**: if they leave home, it alerts and asks if they want to notify someone.
- **Conversational buffer**: "What was I saying?" → "You wanted flan."

## Why is it different?

- **It speaks with the voice your family chooses.** Not a robotic voice. The voice of their son, daughter, grandchild.
- **No internet needed.** It works offline. No data sent to the cloud.
- **No complicated setup.** Family sets it up once.
- **Costs less than 100 USD.** Existing components, no expensive licenses.

## Who is it for?

- People with Alzheimer's or senile dementia.
- Elderly people with diabetic retinopathy (difficulty reading).
- Patients with type 2 diabetes needing continuous monitoring.
- Families caring for a loved one.
- Nursing homes.

## Technology (all existing, all certified)

| Component | Price | Function |
|-----------|-------|----------|
| OV2640 camera | USD 5 | Read labels, recognize faces |
| MPU-6050 gyroscope | USD 2 | Detect falls |
| NFC reader | USD 10 | Connect to FreeStyle Libre 2 |
| ESP32 / nRF53 | USD 5-8 | Local processing |
| Battery | USD 3 | 7-day autonomy |
| Mini speaker | USD 2 | Clear, warm voice |

**Estimated total cost: USD 85**

## Current status

- Prototype in evolution.
- Uses existing certified equipment.
- No additional validation required.
- Complete documentation in the repository.

## License

Copyright © 2026 Enrique Aguayo. All rights reserved.

## Author

Enrique Aguayo H. – Mackiber Labs
