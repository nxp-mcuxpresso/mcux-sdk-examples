# The BSD 3 Clause License
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors
# may be used to endorse or promote products derived from this software without
# specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN if ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_policy(SET CMP0076 NEW)
cmake_policy(SET CMP0079 NEW)

set(NXP_OT_ROOT_PATH "${CMAKE_CURRENT_LIST_DIR}/ot-nxp")
set(NXP_OT_LIBS_PATH "${NXP_OT_ROOT_PATH}/build_rw612/rw612_ot_cli/lib")

if (EXISTS ${NXP_OT_ROOT_PATH})
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
        # rw612 header files
        ${NXP_OT_ROOT_PATH}/src/rw/rw612

        # ot common
        ${NXP_OT_ROOT_PATH}/src/common

        # openthread
        ${NXP_OT_ROOT_PATH}/openthread/src
        ${NXP_OT_ROOT_PATH}/openthread/include
        ${NXP_OT_ROOT_PATH}/openthread/src/core
        ${NXP_OT_ROOT_PATH}/openthread/examples/platforms
        ${NXP_OT_ROOT_PATH}/openthread/third_party/mbedtls
    )
else()
    message(WARNING "Please download ot-nxp in ${CMAKE_CURRENT_LIST_DIR}")
endif()

if (EXISTS ${NXP_OT_LIBS_PATH})
    TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

    # ot ncp libs
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${NXP_OT_LIBS_PATH}/libopenthread-cli-ftd.a
        ${NXP_OT_LIBS_PATH}/libopenthread-ftd.a
        ${NXP_OT_LIBS_PATH}/libopenthread-hdlc.a
        ${NXP_OT_LIBS_PATH}/libopenthread-ncp-ftd.a
        ${NXP_OT_LIBS_PATH}/libopenthread-platform.a
        ${NXP_OT_LIBS_PATH}/libopenthread-rw612.a
        ${NXP_OT_LIBS_PATH}/libopenthread-spinel-ncp.a
        ${NXP_OT_LIBS_PATH}/libot-cli-addons.a
        ${NXP_OT_LIBS_PATH}/libot-cli-rw612.a
        ${NXP_OT_LIBS_PATH}/libopenthread-url.a
        ${NXP_OT_LIBS_PATH}/libopenthread-radio-spinel.a
        ${NXP_OT_LIBS_PATH}/libtcplp-ftd.a
    )

    TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)
else()
    message(WARNING "Please compile ot ncp libs in ${CMAKE_CURRENT_LIST_DIR}")
endif()
