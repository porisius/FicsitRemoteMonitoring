import {
  combineThreeImagesToBase64,
  combineTwoImagesToBase64,
} from "@/lib/helpers";
import { CLASSNAMED_PATH, utils } from "@/public/map/paths";
import { images } from "@/public/map/images";

const resourceClassNames = [
  "Desc_Coal_C",
  "Desc_LiquidOil_C",
  "Desc_NitrogenGas_C",
  "Desc_OreBauxite_C",
  "Desc_OreCopper_C",
  "Desc_OreGold_C",
  "Desc_OreIron_C",
  "Desc_OreUranium_C",
  "Desc_RawQuartz_C",
  "Desc_SAM_C",
  "Desc_Stone_C",
  "Desc_Sulfur_C",
  "Desc_Water_C",
  "Desc_Geyser_C",
];

type IconPair = { x: string; v: string };
type PurityIcons = Record<string, IconPair>;
type Resources = Record<string, PurityIcons>;

const Purities = ["Pure", "Normal", "Impure"] as const;
const colors: Record<(typeof Purities)[number], string> = {
  Pure: "green",
  Normal: "orange",
  Impure: "red",
};

export const generateIcons = async () => {
  let resources: Resources = {};

  for (const resourceClassName of resourceClassNames) {
    let purityIcons: PurityIcons = {};

    for (const purity of Purities) {
      const colorKey = colors[purity] as "green" | "red" | "orange";

      const x: any = await combineThreeImagesToBase64(
        utils.templates.extra[colorKey],
        `${CLASSNAMED_PATH}/${resourceClassName}.avif`,
        utils.templates.extra.x[colorKey],
      );
      const v: any = await combineThreeImagesToBase64(
        utils.templates.extra[colorKey],
        `${CLASSNAMED_PATH}/${resourceClassName}.avif`,
        utils.templates.extra.v[colorKey],
      );

      purityIcons[purity] = { x, v };
    }

    resources[resourceClassName] = purityIcons;
  }

  return {
    Slugs: {
      BP_Crystal_C: await combineTwoImagesToBase64(
        utils.templates.slugs.power_slug_mk1,
        images.Slugs.MK1,
      ),
      BP_Crystal_mk2_C: await combineTwoImagesToBase64(
        utils.templates.slugs.power_slug_mk2,
        images.Slugs.MK2,
      ),
      BP_Crystal_mk3_C: await combineTwoImagesToBase64(
        utils.templates.slugs.power_slug_mk3,
        images.Slugs.MK3,
      ),
    },
    Artifacts: {
      BP_WAT2_C: await combineTwoImagesToBase64(
        utils.templates.artifacts.mercer_sphere,
        images.Artifacts.Mercer_Sphere,
      ),
      BP_WAT1_C: await combineTwoImagesToBase64(
        utils.templates.artifacts.somersloop,
        images.Artifacts.Somersloop,
      ),
    },
    Creatures: {
      Char_SpaceRabbit_C: await combineTwoImagesToBase64(
        utils.templates.creatures.doggo,
        images.Creatures.Space_Rabbit,
      ),
    },
    Drop_Pods: {
      Looted: await combineThreeImagesToBase64(
        utils.templates.extra.green,
        images.Markers.Drop_Pod,
        utils.templates.extra.v.green,
      ),
      Not_Open: await combineThreeImagesToBase64(
        utils.templates.extra.red,
        images.Markers.Drop_Pod,
        utils.templates.extra.x.red,
      ),
      Not_Looted: await combineThreeImagesToBase64(
        utils.templates.extra.white,
        images.Markers.Drop_Pod,
        utils.templates.extra.misc.pickup,
      ),
      Power: await combineThreeImagesToBase64(
        utils.templates.extra.yellow,
        images.Markers.Drop_Pod,
        utils.templates.extra.misc.power,
      ),
    },
    Resources: resources,
    Vehicles: {
      BP_Train_C: {
        Auto: await combineThreeImagesToBase64(
          utils.templates.extra.green,
          images.Markers.Train,
          utils.templates.extra.misc.auto,
        ),
        Manual: await combineThreeImagesToBase64(
          utils.templates.extra.green,
          images.Markers.Train,
          utils.templates.extra.misc.manual,
        ),
        Parked: await combineThreeImagesToBase64(
          utils.templates.extra.yellow,
          images.Markers.Train,
          utils.templates.extra.misc.parked,
        ),
        Derailed: await combineThreeImagesToBase64(
          utils.templates.extra.red,
          images.Markers.Train,
          utils.templates.extra.misc.derailed,
        ),
      },
      BP_Golfcart_C: {
        Auto: await combineThreeImagesToBase64(
          utils.templates.extra.green,
          images.Markers.Factory_Cart,
          utils.templates.extra.misc.auto,
        ),
        Manual: await combineThreeImagesToBase64(
          utils.templates.extra.green,
          images.Markers.Factory_Cart,
          utils.templates.extra.misc.manual,
        ),
        Parked: await combineThreeImagesToBase64(
          utils.templates.extra.yellow,
          images.Markers.Factory_Cart,
          utils.templates.extra.misc.parked,
        ),
        RunningOutOfFuel: await combineThreeImagesToBase64(
          utils.templates.extra.yellow,
          images.Markers.Factory_Cart,
          utils.templates.extra.misc.low_fuel,
        ),
        OutOfFuel: await combineThreeImagesToBase64(
          utils.templates.extra.red,
          images.Markers.Factory_Cart,
          utils.templates.extra.misc.out_of_fuel,
        ),
      },
      BP_Tractor_C: {
        Auto: await combineThreeImagesToBase64(
          utils.templates.extra.green,
          images.Markers.Tractor,
          utils.templates.extra.misc.auto,
        ),
        Manual: await combineThreeImagesToBase64(
          utils.templates.extra.green,
          images.Markers.Tractor,
          utils.templates.extra.misc.manual,
        ),
        Parked: await combineThreeImagesToBase64(
          utils.templates.extra.yellow,
          images.Markers.Tractor,
          utils.templates.extra.misc.parked,
        ),
        RunningOutOfFuel: await combineThreeImagesToBase64(
          utils.templates.extra.yellow,
          images.Markers.Tractor,
          utils.templates.extra.misc.low_fuel,
        ),
        OutOfFuel: await combineThreeImagesToBase64(
          utils.templates.extra.red,
          images.Markers.Tractor,
          utils.templates.extra.misc.out_of_fuel,
        ),
      },
      BP_Truck_C: {
        Auto: await combineThreeImagesToBase64(
          utils.templates.extra.green,
          images.Markers.Truck,
          utils.templates.extra.misc.auto,
        ),
        Manual: await combineThreeImagesToBase64(
          utils.templates.extra.green,
          images.Markers.Truck,
          utils.templates.extra.misc.manual,
        ),
        Parked: await combineThreeImagesToBase64(
          utils.templates.extra.yellow,
          images.Markers.Truck,
          utils.templates.extra.misc.parked,
        ),
        RunningOutOfFuel: await combineThreeImagesToBase64(
          utils.templates.extra.yellow,
          images.Markers.Truck,
          utils.templates.extra.misc.low_fuel,
        ),
        OutOfFuel: await combineThreeImagesToBase64(
          utils.templates.extra.red,
          images.Markers.Truck,
          utils.templates.extra.misc.out_of_fuel,
        ),
      },
      BP_Explorer_C: {
        Auto: await combineThreeImagesToBase64(
          utils.templates.extra.green,
          images.Markers.Explorer,
          utils.templates.extra.misc.auto,
        ),
        Manual: await combineThreeImagesToBase64(
          utils.templates.extra.green,
          images.Markers.Explorer,
          utils.templates.extra.misc.manual,
        ),
        Parked: await combineThreeImagesToBase64(
          utils.templates.extra.yellow,
          images.Markers.Explorer,
          utils.templates.extra.misc.parked,
        ),
        RunningOutOfFuel: await combineThreeImagesToBase64(
          utils.templates.extra.yellow,
          images.Markers.Explorer,
          utils.templates.extra.misc.low_fuel,
        ),
        OutOfFuel: await combineThreeImagesToBase64(
          utils.templates.extra.red,
          images.Markers.Explorer,
          utils.templates.extra.misc.out_of_fuel,
        ),
      },
      BP_DroneTransport_C: {
        No_Route: await combineThreeImagesToBase64(
          utils.templates.extra.red,
          images.Markers.Drone,
          utils.templates.extra.misc.no_route,
        ),
        No_Destination: await combineThreeImagesToBase64(
          utils.templates.extra.red,
          images.Markers.Drone,
          utils.templates.extra.misc.no_destination,
        ),
        Parked: await combineThreeImagesToBase64(
          utils.templates.extra.yellow,
          images.Markers.Drone,
          utils.templates.extra.misc.parked,
        ),
        Flying: await combineThreeImagesToBase64(
          utils.templates.extra.green,
          images.Markers.Drone,
          utils.templates.extra.misc.flying,
        ),
        Travelling: await combineThreeImagesToBase64(
          utils.templates.extra.green,
          images.Markers.Drone,
          utils.templates.extra.misc.travelling,
        ),
      },
    },
    Buildings: {
      Build_TrainStation_C: await combineTwoImagesToBase64(
        utils.templates.extra.green,
        images.Markers.Train_Station,
      ),
      Build_DroneStation_C: await combineTwoImagesToBase64(
        utils.templates.extra.green,
        images.Markers.Drone_Station,
      ),
      Build_TruckStation_C: await combineTwoImagesToBase64(
        utils.templates.extra.green,
        images.Markers.Truck_Station,
      ),
      Build_HubTerminal_C: {
        No_Milestone: await combineThreeImagesToBase64(
          utils.templates.extra.green,
          images.Markers.Hub,
          utils.templates.extra.misc.no_milestone,
        ),
        Ship_Returning: await combineThreeImagesToBase64(
          utils.templates.extra.yellow,
          images.Markers.Hub,
          utils.templates.extra.misc.ship_returning,
        ),
        Normal: await combineTwoImagesToBase64(
          utils.templates.default,
          images.Markers.Hub,
        ),
      },
      Build_Portal_C: await combineTwoImagesToBase64(
        utils.templates.default,
        images.Markers.Portal,
      ),
      Build_PortalSatellite_C: await combineTwoImagesToBase64(
        utils.templates.default,
        images.Markers.Portal_Satellite,
      ),
      Build_SpaceElevator_C: {
        Upgrade_Not_Ready: await combineThreeImagesToBase64(
          utils.templates.extra.red,
          images.Markers.Space_Elevator,
          utils.templates.extra.misc.upgrade_not_ready,
        ),
        Upgrade_Ready: await combineThreeImagesToBase64(
          utils.templates.extra.yellow,
          images.Markers.Space_Elevator,
          utils.templates.extra.misc.upgrade_ready,
        ),
        Fully_Upgraded: await combineThreeImagesToBase64(
          utils.templates.extra.green,
          images.Markers.Space_Elevator,
          utils.templates.extra.v.green,
        ),
      },
      Build_RadarTower_C: await combineTwoImagesToBase64(
        utils.templates.default,
        images.Markers.Radar_Tower,
      ),
    },
    Players: {
      Alive: await combineThreeImagesToBase64(
        utils.templates.extra.green,
        images.Markers.Player,
        utils.templates.extra.v.green,
      ),
      Dead: await combineThreeImagesToBase64(
        utils.templates.extra.red,
        images.Markers.Player_Dead,
        utils.templates.extra.x.red,
      ),
      Offline: await combineThreeImagesToBase64(
        utils.templates.extra.red,
        images.Markers.Player_Offline,
        utils.templates.extra.x.red,
      ),
    },
    Unlock_Items: {
      Tape: await combineThreeImagesToBase64(
        utils.templates.extra.white,
        images.Utils.Lock,
        utils.templates.extra.misc.tape,
        40,
      ),
      Customization: await combineThreeImagesToBase64(
        utils.templates.extra.white,
        images.Utils.Lock,
        utils.templates.extra.misc.customization,
        40,
      ),
      Unlock: await combineThreeImagesToBase64(
        utils.templates.extra.white,
        images.Utils.Lock,
        utils.templates.extra.misc.unlock,
        40,
      ),
    },
  };
};

export const mapMarkers = {
  329: images.MonochromeIcons.Arrow_Down,
  330: images.MonochromeIcons.Arrow_DownLeft,
  331: images.MonochromeIcons.Arrow_DownRight,
  332: images.MonochromeIcons.Arrow_Left,
  333: images.MonochromeIcons.Arrow_UpLeft,
  334: images.MonochromeIcons.Arrow_Up,
  335: images.MonochromeIcons.Arrow_UpRight,
  336: images.MonochromeIcons.Arrow_Right,
  337: images.MonochromeIcons.Drop,
  338: images.MonochromeIcons.Radiation,
  339: images.MonochromeIcons.ThumbUp,
  340: images.MonochromeIcons.ThumbDown,
  341: images.MonochromeIcons.Stop_X,
  342: images.MonochromeIcons.RoadArrow_Down,
  343: images.MonochromeIcons.RoadArrow_TurnAround,
  344: images.MonochromeIcons.RoadArrow_TurnLeft,
  345: images.MonochromeIcons.RoadArrow_TurnRight,
  346: images.MonochromeIcons.RoadArrow_Up,
  347: images.MonochromeIcons.Crate,
  348: images.MonochromeIcons.Exit,
  349: images.MonochromeIcons.Factory,
  350: images.MonochromeIcons.Home,
  351: images.MonochromeIcons.Pioneer,
  352: images.MonochromeIcons.Power,
  353: images.MonochromeIcons.Tractor,
  354: images.MonochromeIcons.Explorer,
  355: images.MonochromeIcons.Truck,
  356: images.MonochromeIcons.Train,
  357: images.MonochromeIcons.Factory_Cart,
  358: images.MonochromeIcons.Drone,
  362: images.MonochromeIcons.Warning,
  598: images.MonochromeIcons.Check,
  644: images.MonochromeIcons.Recycle,
  645: images.MonochromeIcons.Trash,
  650: images.Stamps.Biomass,
  651: images.Stamps.Cave,
  652: images.Stamps.Crash,
  653: images.Stamps.Crate,
  654: images.Stamps.Creature,
  656: images.Stamps["Question Mark"],
  657: images.Stamps.Fluids,
  659: images.Stamps.Radiation,
  660: images.Stamps.Rock,
  661: images.Stamps.Warning,
  662: images.Stamps.Fruit,
  663: images.Stamps.Slug,
  782: images.MonochromeIcons.Question_Mark,
};
