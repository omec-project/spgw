License & Copyright
----

SPDX-License-Identifier: Apache-2.0

SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

Copyright (c) 2020 Selfie Networks, Inc


# SPGW-C Rapid Iteration Development Environment Setup

## High Level Overview

* Update the helm chart to mount the source directory on the host, and to run a
  no-op binary in the container
* Build a development docker image that includes the toolchain so we can
  rebuild the source inside the container
* Build the third party components within the bind-mounted host directory
* Build spgwc

## Detailed Steps

### Update the helm chart

* Add a volume mount for the source
* Replace the container command with 'sleep infinity'
* Replace the health checks with no-ops to /bin/true

  ```diff
  aaron@aether-test-2:~$ diff -u a/statefulset-spgwc.yaml b/statefulset-spgwc.yaml
  --- a/statefulset-spgwc.yaml    2020-10-21 17:22:47.946514099 +0000
  +++ b/statefulset-spgwc.yaml    2020-10-21 17:24:27.303851112 +0000
  @@ -48,27 +48,23 @@
         {{- end }}
           stdin: true
           tty: true
  -        command: ["/opt/cp/scripts/spgwc-run.sh"]
  +        command:
  +          - sleep
  +        args:
  +          - infinity
           livenessProbe:
  -          httpGet:
  -            path: /liveness
  -            port: {{ .Values.config.spgwc.rest.port }}
  -          initialDelaySeconds: 10
  -          periodSeconds: 3
  +          exec:
  +            command:
  +              - /bin/true
           readinessProbe:
  -          httpGet:
  -            path: /readiness
  -            port: {{ .Values.config.spgwc.rest.port }}
  -          initialDelaySeconds: 10
  -          periodSeconds: 3
  +          exec:
  +            command:
  +              - /bin/true
   {{- if semverCompare ">=1.16-0" .Capabilities.KubeVersion.GitVersion }}
           startupProbe:
  -          #looks like available only in 1.16 K8s version and above
  -          httpGet:
  -            path: /startup
  -            port: {{ .Values.config.spgwc.rest.port }}
  -          failureThreshold: 30
  -          periodSeconds: 10
  +          exec:
  +            command:
  +              - /bin/true
   {{- end }}
           env:
           - name: MEM_LIMIT
  @@ -91,6 +87,8 @@
             subPath: spgwc-run.sh
           - name: cp-config
             mountPath: /etc/cp/config
  +        - name: cp-dev
  +          mountPath: /spgw
         {{- if .Values.config.coreDump.enabled }}
           - name: coredump
             mountPath: /tmp/coredump
  @@ -104,6 +102,9 @@
           configMap:
             name: spgwc
             defaultMode: 420
  +      - name: cp-dev
  +        hostPath:
  +          path: /home/aaron/omec-project/spgw
       {{- if .Values.config.coreDump.enabled }}
         - name: host-rootfs
           hostPath:
  ```

### Update the Docker file

* Add a new target called 'docker-image-dev' for the development image

  ```diff
  diff --git a/Dockerfile b/Dockerfile
  index 6162a2c..346ff65 100644
  --- a/Dockerfile
  +++ b/Dockerfile
  @@ -49,6 +49,11 @@ RUN source ./docker-scripts/install_builddeps.sh && \
       source ./docker-scripts/build_spgw.sh && \
       build_spgw

  +FROM spgw as spgw-dev
  +SHELL ["/bin/bash", "-c"]
  +COPY ./docker-scripts/install_rundeps.sh .
  +RUN source ./install_rundeps.sh && install_run_cp_deps && cleanup_image
  +
   FROM $BASE_OS as runtime
   SHELL ["/bin/bash", "-c"]
   COPY ./docker-scripts/install_rundeps.sh .
  ```

### Deploy

* Run `make docker-image-dev`
* Run `make -C ~/cord/aether-in-a-box` (or wherever this lives on your machine)

### Prepare environment

* Enter the container

  ```shell
  kubectl exec -ti -n omec spgwc-0 -- bash
  ```

* Initialize the environment (needs done every time you enter the container):

  ```shell
  export RTE_MACHINE=native
  export EXTRA_CFLAGS='-ggdb -O0'
  source ./docker-scripts/install_builddeps.sh
  source ./docker-scripts/install_hyperscan.sh
  source ./docker-scripts/install_webutils.sh
  source ./docker-scripts/install_dpdk.sh
  source ./docker-scripts/install_oss_util.sh
  source ./docker-scripts/build_spgw.sh
  alias build_spgw_quick="make -j10 -C /spgw clean-cp build-cp RTE_MACHINE='$RTE_MACHINE' EXTRA_FLAGS='$EXTRA_CFLAGS'"
  ```

* Install dependencies.  This needs done before first build.  The third_party
  directory gets created as part of the docker image, but when bind-mounting a
  host directory, third_party does not exist, and the components need to be
  built in place.  Once the dependencies are built, this step can be skipped.

  ```shell
  install_builddeps && \
  install_dpdk && \
  install_hyperscan && \
  build_prometheus && \
  build_pistache && \
  install_oss_util
  ```

### Building

* If not already done so, run the `Initialize the environment` step to
  initialize the environment

* Initiate the build.  This will build everything including the cpp-iface and
  takes about 20 seconds on an 8-core machine:

  ```shell
  build_spgw
  ```

* If only `cp/` needs to be compiled, skip the other components and just build
  cp:

  ```shell
  build_spgw_quick
  ```

* Note: the build-cp target doesn't do dependency detection properly, so
  `clean-cp` should be run between builds.  Build time could be dropped to <1s
  if this was fixed.

* To run, the libraries in the build directory need to be linked.

  ```shell
  for I in `find /spgw -name *.so`; do ln -sf $I /usr/lib; done
  ```

### Running & Debugging

* To run SPGW-C interactively, run it from the 'cp' subdirectory so the
  hard-coded configuration paths are ignored (even though `-f /etc/cp/config/`
  is specified, restartCnt.txt ignores this path & uses a relative path from
  the working directory and spgw-c will crash on startup if it cannot find this
  file):

  ```shell
  cd /spgw/cp && build/ngic_controlplane -c ff --no-huge -m 30555 --no-pci -- -f /etc/cp/config/
  ```

* To use gdb, do the following:

  ```shell
  cd /spgw/cp
  gdb build/ngic_controlplane
  r -c ff --no-huge -m 30555 --no-pci -- -f /etc/cp/config/
  ```
