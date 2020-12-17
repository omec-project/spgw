# SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
# Copyright (c) 2017 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0
# SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

DIRS-y = lib cpplib libgtpv2c libpfcp cp test

#define targets
CLEANDIRS-y = $(DIRS-y:%=clean-%)
BUILDIRS-y = $(DIRS-y:%=build-%)

all: $(BUILDIRS-y)

$(BUILDIRS-y):
	$(MAKE) -C $(@:build-%=%)

clean: $(CLEANDIRS-y)

$(CLEANDIRS-y):
	$(MAKE) -C $(@:clean-%=%) clean

VERSION                  ?= $(shell cat ./VERSION)
DOCKER_TAG               ?= ${VERSION}
DOCKER_REGISTRY          ?=
DOCKER_REPOSITORY        ?=
DOCKER_BUILDKIT          ?= 1
DOCKER_VERSION            = $(shell docker --version | awk '{ print $3 }' | cut -c1-5)
# Note that we set the target platform of Docker images to Haswell
# so that the images work on any platforms with Haswell CPUs or newer.
# To get the best performance optimization to your target platform,
# please build images on the target machine with RTE_MACHINE='native'.
DOCKER_BUILD_ARGS        ?= --build-arg RTE_MACHINE='hsw'

## Docker labels. Only set ref and commit date if committed
DOCKER_LABEL_VCS_URL     ?= $(shell git remote get-url $(shell git remote))
DOCKER_LABEL_VCS_REF     ?= $(shell git diff-index --quiet HEAD -- && git rev-parse HEAD || echo "unknown")
DOCKER_LABEL_COMMIT_DATE ?= $(shell git diff-index --quiet HEAD -- && git show -s --format=%cd --date=iso-strict HEAD || echo "unknown" )
DOCKER_LABEL_BUILD_DATE  ?= $(shell date -u "+%Y-%m-%dT%H:%M:%SZ")

# https://docs.docker.com/engine/reference/commandline/build/#specifying-target-build-stage---target
docker-build:
	# Enable compatibility with Docker versions without the --progress flag.
	if [ $(echo "$DOCKER_VERSION >= 18.09" | bc -l) ];\
		then PROGRESS_TAG="--progress=plain";\
		else PROGRESS_TAG="--progress=plain";\
	fi

	DOCKER_BUILDKIT=$(DOCKER_BUILDKIT) docker build --pull --progress=plain $(PROGRESS_TAG) $(DOCKER_BUILD_ARGS) \
		--tag ${DOCKER_REGISTRY}${DOCKER_REPOSITORY}spgw:${DOCKER_TAG} \
		--label "org.label-schema.schema-version=1.0" \
		--label "org.label-schema.name=spgw-$$target" \
		--label "org.label-schema.version=${VERSION}" \
		--label "org.label-schema.vcs-url=${DOCKER_LABEL_VCS_URL}" \
		--label "org.label-schema.vcs-ref=${DOCKER_LABEL_VCS_REF}" \
		--label "org.label-schema.build-date=${DOCKER_LABEL_BUILD_DATE}" \
		--label "org.opencord.vcs-commit-date=${DOCKER_LABEL_COMMIT_DATE}" \
		.;

docker-push:
	docker push ${DOCKER_REGISTRY}${DOCKER_REPOSITORY}spgw:${DOCKER_TAG};

.PHONY: $(RECURSIVETARGETS) $(WHAT) $(CPDEPS) docker-build docker-push
.SILENT: docker-build docker-push
