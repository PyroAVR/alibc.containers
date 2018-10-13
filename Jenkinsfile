pipeline {
  agent {
    docker {
      image 'aperture_standard'
    }

  }
  stages {
    stage('clone') {
      steps {
        git(url: 'https://build.apunl.org/stash/scm/alc/extensions.git', branch: 'master')
        dir(path: 'extensions')
      }
    }
    stage('configure') {
      steps {
        sh '''mkdir build
meson build
'''
      }
    }
    stage('build') {
      steps {
        dir(path: 'build')
        sh '''ninja
'''
      }
    }
    stage('package') {
      steps {
        archiveArtifacts(artifacts: '*', allowEmptyArchive: true, caseSensitive: true, onlyIfSuccessful: true)
      }
    }
  }
}