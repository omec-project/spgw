# Copyright (c) 2017 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

DIRS-y =  libgtpv2c libpfcp cp dp test

#define targets
CLEANDIRS-y = $(DIRS-y:%=clean-%)
BUILDIRS-y = $(DIRS-y:%=build-%)

all: $(BUILDIRS-y)

$(BUILDIRS-y):
	$(MAKE) -C $(@:build-%=%)

clean: $(CLEANDIRS-y)

$(CLEANDIRS-y):
	$(MAKE) -C $(@:clean-%=%) clean

RECURSIVETARGETS := all clean
CPDEPS := libgtpv2c
DPDEPS := lib
DIRS := cp dp test
# Use 'make WHAT=cp' to compile cp only
WHAT ?= $(DIRS)

$(RECURSIVETARGETS): $(WHAT)
$(CPDEPS) $(DPDEPS):
	$(MAKE) -C $@ $(MAKECMDGOALS)
cp: $(CPDEPS)
	$(MAKE) -C $@ $(MAKECMDGOALS)
dp: $(DPDEPS)
	$(MAKE) -C $@ $(MAKECMDGOALS)
test: $(CPDEPS) $(DPDEPS)
	$(MAKE) -C $@ $(MAKECMDGOALS)

VERSION                  ?= $(shell cat ./VERSION)
DOCKER_TAG               ?= ${VERSION}
DOCKER_DEBUG_TAG         ?= ${DOCKER_TAG}-debug
DOCKER_REGISTRY          ?=
DOCKER_REPOSITORY        ?=
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
DOCKER_TARGETS           ?= basepkg dpdkbuild freediambuild hyperscanbuild ossutil

# https://docs.docker.com/engine/reference/commandline/build/#specifying-target-build-stage---target
docker-build:
	for target in $(DOCKER_TARGETS); do \
		docker build $(DOCKER_BUILD_ARGS) \
			--target $$target \
			--tag ${DOCKER_REGISTRY}${DOCKER_REPOSITORY}ngic-$$target:${DOCKER_TAG} \
			--label "org.label-schema.schema-version=1.0" \
			--label "org.label-schema.name=ngic-$$target" \
			--label "org.label-schema.version=${VERSION}" \
			--label "org.label-schema.vcs-url=${DOCKER_LABEL_VCS_URL}" \
			--label "org.label-schema.vcs-ref=${DOCKER_LABEL_VCS_REF}" \
			--label "org.label-schema.build-date=${DOCKER_LABEL_BUILD_DATE}" \
			--label "org.opencord.vcs-commit-date=${DOCKER_LABEL_COMMIT_DATE}" \
			.; \
	done

docker-push:
	for target in $(DOCKER_TARGETS); do \
		docker push ${DOCKER_REGISTRY}${DOCKER_REPOSITORY}ngic-$$target:${DOCKER_TAG}; \
		docker push ${DOCKER_REGISTRY}${DOCKER_REPOSITORY}ngic-$$target:${DOCKER_DEBUG_TAG}; \
	done

.PHONY: $(RECURSIVETARGETS) $(WHAT) $(CPDEPS) $(DPDEPS) docker-build docker-push
.SILENT: docker-build docker-push
