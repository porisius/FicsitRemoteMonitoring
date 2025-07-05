export const BASE_PATH = "/map/";

const NORMAL_PATH = `${BASE_PATH}normal/`;
export const ITEMS_PATH = `${NORMAL_PATH}items/`;
export const MARKERS_PATH = `${NORMAL_PATH}markers/`;
export const STAMPS_PATH = `${NORMAL_PATH}stamps/`;
export const MONOCHROMEICONS_PATH = `${NORMAL_PATH}MonochromeIcons/`;

export const CLASSNAMED_PATH = `${BASE_PATH}classnamed/`;

export const UTILS_PATH = `${BASE_PATH}utils/`;

const TEMPLATES_PATH = `${UTILS_PATH}templates/`;
const CREATURES_PATH = `${TEMPLATES_PATH}creatures/`;
const ARTIFACTS_PATH = `${TEMPLATES_PATH}artifacts/`;
const SLUGS_PATH = `${TEMPLATES_PATH}slugs/`;

const EXTRA_PATH = `${TEMPLATES_PATH}extra/`;
const V_PATH = `${EXTRA_PATH}v/`;
const X_PATH = `${EXTRA_PATH}x/`;
const MISC_PATH = `${EXTRA_PATH}misc/`;

export const utils = {
  templates: {
    slugs: {
      power_slug_mk1: `${SLUGS_PATH}power_slug_mk1.avif`,
      power_slug_mk2: `${SLUGS_PATH}power_slug_mk2.avif`,
      power_slug_mk3: `${SLUGS_PATH}power_slug_mk3.avif`,
    },
    artifacts: {
      mercer_sphere: `${ARTIFACTS_PATH}mercer_sphere.avif`,
      somersloop: `${ARTIFACTS_PATH}somersloop.avif`,
    },
    creatures: {
      doggo: `${CREATURES_PATH}doggo.avif`,
    },
    extra: {
      green: `${EXTRA_PATH}green.avif`,
      orange: `${EXTRA_PATH}orange.avif`,
      red: `${EXTRA_PATH}red.avif`,
      yellow: `${EXTRA_PATH}yellow.avif`,
      white: `${EXTRA_PATH}white.avif`,
      v: {
        green: `${V_PATH}green.avif`,
        orange: `${V_PATH}orange.avif`,
        red: `${V_PATH}red.avif`,
      },
      x: {
        green: `${X_PATH}green.avif`,
        orange: `${X_PATH}orange.avif`,
        red: `${X_PATH}red.avif`,
      },
      misc: {
        power: `${MISC_PATH}power.avif`,
        pickup: `${MISC_PATH}pickup.avif`,
        auto: `${MISC_PATH}auto.avif`,
        manual: `${MISC_PATH}manual.avif`,
        loading: `${MISC_PATH}loading.avif`,
        unloading: `${MISC_PATH}unloading.avif`,
        paused: `${MISC_PATH}paused.avif`,
        derailed: `${MISC_PATH}derailed.avif`,
        parked: `${MISC_PATH}parked.avif`,
        out_of_fuel: `${MISC_PATH}out_of_fuel.avif`,
        low_fuel: `${MISC_PATH}low_fuel.avif`,
        no_route: `${MISC_PATH}no_route.avif`,
        no_destination: `${MISC_PATH}no_destination.avif`,
        flying: `${MISC_PATH}flying.avif`,
        travelling: `${MISC_PATH}travelling.avif`,
        ship_returning: `${MISC_PATH}ship_returning.avif`,
        no_milestone: `${MISC_PATH}no_milestone.avif`,
        upgrade_not_ready: `${MISC_PATH}upgrade_not_ready.avif`,
        upgrade_ready: `${MISC_PATH}upgrade_ready.avif`,
        customization: `${MISC_PATH}customization.avif`,
        open: `${MISC_PATH}open.avif`,
        tape: `${MISC_PATH}tape.avif`,
        unlock: `${MISC_PATH}unlock.avif`,
      },
    },
    default: `${UTILS_PATH}default.avif`,
  },
};
