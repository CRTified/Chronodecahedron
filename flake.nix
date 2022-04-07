{
  description = ''
    Service package and configuration module for the Chronodecahedron time tracker.
  '';
  inputs = { nixpkgs.url = "github:NixOS/nixpkgs"; };
  outputs = { self, nixpkgs }: {
    packages = builtins.listToAttrs (map (system: {
      name = system;
      value = let pkgs = import nixpkgs { inherit system; };
      in {
        chronodecahedron-watcher =
          pkgs.writers.writePython3Bin "chronodecahedron-watcher" {
            libraries = with pkgs.python3Packages; [ paho-mqtt ];
          } (builtins.readFile ./src/chronodecahedron-watcher.py);
      };
    }) nixpkgs.legacyPackages.x86_64-linux.python3.meta.platforms);

    nixosModule = { config, ... }:
      let cfg = config.services.chronodecahedron-watcher;
      in with nixpkgs.lib; {
        # Interface definition
        options.services.chronodecahedron-watcher = {
          enable = mkEnableOption "Chronodecahedron watcher";

          user = mkOption {
            type = types.str;
            default = "root";
            description = "User account under which the watcher runs.";
          };
          group = mkOption {
            type = types.str;
            default = "root";
            description = "Group under which the watcher runs.";
          };

          mqttServer = mkOption {
            type = types.str;
            default = "localhost";
            description = "MQTT server hostname.";
          };
          mqttPort = mkOption {
            type = types.ints.u16;
            default = 1883;
            description = "MQTT server port.";
          };
          mqttCredentialsFile = mkOption {
            type = types.path;
            default = null;
            description = ''
              Path to a file containing the environment variables 
              <literal>CHRONO_MQTT_USER</literal> and
              <literal>CHRONO_MQTT_PASS</literal>.
              If set, these are used for authentication against the MQTT server.
            '';
          };

          logFile = mkOption {
            type = types.path;
            default = null;
            example = "/root/chronodecahedron_log.txt";
            description = ''
              Path to the persistent logfile. 
              If null, the watcher will only log to stdout.
            '';
          };

          device = mkOption {
            type = types.str;
            default = "timecube";
            description = ''
              Hostname of the chronodecahedron. Used as part of the
              MQTT topic.
            '';
          };
        };

        # Implementation
        config = mkIf cfg.enable {
          systemd.services.chronodecahedron-watcher = {
            description = "Chronodecahedron MQTT watcher";
            wantedBy = [ "multi-user.target" ];
            after = [ "networking.target" ];

            script = ''
              ${
                self.packages."${config.nixpkgs.system}".chronodecahedron-watcher
              }/bin/chronodecahedron-watcher \
                --server "${cfg.mqttServer}" \
                --port ${toString cfg.mqttPort} \
                --device "${cfg.device}" \
              '' + (optionalString (cfg.logFile != null)
              ''  --logfile "${cfg.logFile}"'');

            serviceConfig = {
              Restart = "always";
              User = cfg.user;
              Group = cfg.group;
              EnvironmentFile = cfg.mqttCredentialsFile;
            };
          };
        };
      };
  };
}
