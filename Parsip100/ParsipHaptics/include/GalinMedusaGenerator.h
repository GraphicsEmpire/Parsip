// Ryan Schmidt   rms@unknownroad.com
// Copyright (c) 2007. All Rights Reserved
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
// OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// This source code is provided for non-commercial,
// academic use only. It may not be disclosed or distributed,
// in part or in whole, without the express written consent
// of the Copyright Holder (Ryan Schmidt). This copyright notice must
// not be removed from any original or modified source files, and 
// must be included in any source files which contain portions of
// the original source code.


#ifndef __RMS_GALIN_MEDUSA_GENERATOR_H__
#define __RMS_GALIN_MEDUSA_GENERATOR_H__

#include <vector>
#include "PS_FrameWork/include/PS_Vector.h"

#include "PS_BlobTree/include/CBlend.h"

using namespace PS::BLOBTREE;

namespace PS {

class GalinMedusaGenerator {
public:
        static CBlend * Medusa_Neck();
        static CBlend * Medusa_Tail();
        static CBlend * Medusa_Body();
        static CBlend * Medusa_LeftHand();
        static CBlend * Medusa_Breast();
        static CBlend * Medusa_Hair();

	// [RMS NOTE: this was not ported exactly. see code..]
        static CBlend * Medusa_Tete();
};


}  // end namespace rmsimplicit


#endif // __RMS_GALIN_MEDUSA_GENERATOR_H__
